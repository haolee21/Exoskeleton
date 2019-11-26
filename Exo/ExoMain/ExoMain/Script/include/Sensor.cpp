#include "Sensor.h"
#include <memory>

#define SENSOR_PRIORITY (99)             /* we use 49 as the PRREMPT_RT use 50 \
                                        as the priority of kernel tasklets \
                                        and interrupt handler by default */
#define POOLSIZE (200 * 1024 * 1024) // 200MB
#define MAX_SAFE_STACK (100 * 1024)  /* The maximum stack size which is \
                                      guranteed safe to access without  \
                                      faulting */



typedef std::chrono::duration<long, std::nano> nanosecs_t;
typedef std::chrono::duration<int, std::micro> microsecs_t;
typedef std::chrono::duration<unsigned long, std::micro> microsecs_ul;
typedef std::chrono::duration<int, std::milli> millisecs_t;
Sensor::Sensor(std::string _filePath,char *portName, long sampT,std::shared_ptr<Com> _com,bool _display)
{
	this->ResetPin.reset(new Pin(SYNCOUT));
	this->display = _display;
	std::cout << "creating" << endl;
	
	if (!this->is_create)
	{
		this->filePath = _filePath;
		std::cout << "Create Sensor" << endl;
		this->serialDevId = this->serialPortConnect(portName);
		this->sampT = sampT*USEC;
		
		//initialize data receiving buffer
		this->frontBuf_count.reset(new int(0));
		this->backBuf_count.reset(new int(0));
		this->frontBuf.reset(new char[DATALEN]);
		this->backBuf.reset(new char[DATALEN]);
		this->senDataLock.reset(new std::mutex);

		this->frontBuf_ptr = this->frontBuf.get();
		this->backBuf_ptr = this->backBuf.get();
		this->init = false;
		this->falseSenCount = 0;

		this->senData.reset(new int[NUMSEN + 1]);
		this->oriData.reset(new int[NUMSEN]);
		this->senDataRaw.reset(new char[DATALEN]);

		memset(this->senData.get(), 0, NUMSEN + 1);
		memset(this->serialBuf,'\0',SIZEOFBUFFER);
		memset(this->senDataRaw.get(), '\0', DATALEN);
		if (this->serialDevId == -1)
			std::cout << "Sensor init failed" << endl;


		
		this->senRec.reset(new Recorder<int>("sen",_filePath,"Time,LHipPos,LKnePos,LAnkPos,RHipPos,RKnePos,RAnkPos,sen7,sen8,TankPre,LKnePre,LAnkPre,RKnePre,RAnkPre,sen14,sen15,sen16"));
		//Controller
		this->com = _com.get();
		
	}
	else
		std::cout << "Sensor already created" << endl;
}

void Sensor::Start(std::chrono::system_clock::time_point startTime)
{
	this->ResetPin->On();
	this->origin = startTime;
	this->sw_senUpdate = true;
	
	//initialize the butterworth filter 
	
	
	//this->th_SenUpdate.reset(new thread(&Sensor::senUpdate, this));

	struct sched_param param;
	Common::initialize_memory_allocation();
	param.sched_priority = SENSOR_PRIORITY;

	if (sched_setscheduler(0, SCHED_FIFO, &param) == -1){
		perror("sched_setscheduler failed");
        exit(-1);
    }
	if (mlockall(MCL_CURRENT | MCL_FUTURE) == -1){
        perror("mlockall failed");
        exit(-2);
    }

    /* Pre-fault our stack */
	Common::stack_prefault();


	this->ResetPin->Off();
	// if(pthread_attr_init(&this->attr))
	// 	exo_error(1);
	// if(pthread_attr_setstacksize(&this->attr,PTHREAD_STACK_MIN+MY_STACK_SIZE))
	// 	exo_error(2);
	pthread_attr_init(&this->attr);
	pthread_attr_setstacksize(&this->attr,PTHREAD_STACK_MIN+MY_STACK_SIZE);
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	pthread_create(&this->th_SenUpdate,&this->attr, &Sensor::senUpdate,this);
	std::cout << "initial receiving thread" << endl;
	
}
void Sensor::Stop()
{
	std::cout << "get into stop" << endl;
	{
		std::lock_guard<mutex> lock(this->senUpdateLock);
		this->sw_senUpdate = false;
	}
	std::cout << "sen stop flag on\n";
	//this->sw_senUpdate = false;
	pthread_join(this->th_SenUpdate,NULL);
	std::cout << "sen loop joined\n";
	this->serialPortClose(this->serialDevId);
	//this->th_SenUpdate->join();
	
	std::cout<<"sensor fully stops\n";
	
}
//timer
// void Sensor::tsnorm(struct timespec *ts)
// {
//     while (ts->tv_nsec >= NSEC_PER_SEC)
//     {
//         ts->tv_nsec -= NSEC_PER_SEC;
//         ts->tv_sec++;
//     }
// }
//
void *Sensor::senUpdate(void *_sen)
{
	Sensor *sen = (Sensor*) _sen;
	Controller *con = new Controller(sen->filePath,sen->com,sen->display,sen->origin,sen->sampT);
	std::unique_ptr<std::thread> conTh;
	bool conStart = false;

	//for accurate timer
	
    
    
    

	struct timespec t;
	
	

   
	int flushVal = tcflush(sen->serialDevId, TCIFLUSH);
	if(flushVal==0){
		std::cout << "clear buffer\n";
	}
	else{
		std::cout << "failed to clear the buffer\n";
	}
	sen->ResetPin->Off();
	bool curSenCond = true;
	while (true)
	{	
		//timer
		if(sen->senNotInit){
			sen->senNotInit=false;
			clock_gettime(CLOCK_MONOTONIC, &t);
		}
		//

		sen->readSerialPort(sen->serialDevId);
		
		if (conStart)
		{
			// if(conTh->joinable()){ //I am not sure but the program stop to freeze after I put this 
			// 	conTh->join();
			// }
			
		}
		else{
			conStart = true;
			con->Start(sen->senData.get(), sen->senDataRaw.get(), sen->senDataLock.get());
		}
		//conTh.reset(new std::thread(&Controller::ConMainLoop,con,sen->senData.get(),sen->senDataRaw.get()));
		

		{
			std::lock_guard<std::mutex> lock(sen->senUpdateLock);
			curSenCond = sen->sw_senUpdate;
		}
		if (!curSenCond)
		{
			break;
		}
		

		// timer
		// calculate next shot
		
		
        t.tv_nsec += sen->sampT;
        Common::tsnorm(&t);
		clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t, NULL);
		//clock_gettime(CLOCK_MONOTONIC, &t);
	}
	std::cout<<"end sampling\n";
	con->Stop();
	std::cout << "sensor ends" << endl;
	sen->saveData_th.reset(new std::thread(&Sensor::SaveAllData,sen)); //original purpose is for some reason, sen is destoried before it went through this line
	sen->saveData_th->join();
	//sen->senRec.reset();
	delete con;

	return 0;
}
void Sensor::SaveAllData(){
	this->senRec.reset();
}




//reference from https://blog.mbedded.ninja/programming/operating-systems/linux/linux-serial-ports-using-c-cpp/

int Sensor::serialPortConnect(char *portName)
{

	// Open the serial port. Change device path as needed (currently set to an standard FTDI USB-UART cable type device)
	int serial_port = open(portName, O_RDWR);

	// Create new termios struc, we call it 'tty' for convention
	struct termios tty;
	memset(&tty, 0, sizeof tty);

	// Read in existing settings, and handle any error
	if (tcgetattr(serial_port, &tty) != 0)
	{
		printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
	}

	tty.c_cflag |= PARENB;		   // set parity bit, disabling parity (most common)
	tty.c_cflag &= ~CSTOPB;		   // Clear stop field, only one stop bit used in communication (most common)
	tty.c_cflag |= CS8;			   // 8 bits per byte (most common)
	tty.c_cflag &= ~CRTSCTS;	   // Disable RTS/CTS hardware flow control (most common)
	tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)

	tty.c_lflag &= ~ICANON;
	tty.c_lflag &= ~ECHO;														 // Disable echo
	tty.c_lflag &= ~ECHOE;														 // Disable erasure
	tty.c_lflag &= ~ECHONL;														 // Disable new-line echo
	tty.c_lflag &= ~ISIG;														 // Disable interpretation of INTR, QUIT and SUSP
	tty.c_iflag &= ~(IXON | IXOFF | IXANY);										 // Turn off s/w flow ctrl
	tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL); // Disable any special handling of received bytes

	tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
	tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed
	// tty.c_oflag &= ~OXTABS; // Prevent conversion of tabs to spaces (NOT PRESENT ON LINUX)
	// tty.c_oflag &= ~ONOEOT; // Prevent removal of C-d chars (0x004) in output (NOT PRESENT ON LINUX)

	tty.c_cc[VTIME] = 0; // Wait for up to 1s (10 deciseconds), returning as soon as any data is received.
	tty.c_cc[VMIN] = 1;

	// Set in/out baud rate to be 115200
	cfsetispeed(&tty, B460800);
	cfsetospeed(&tty, B460800);

	// Save tty settings, also checking for error
	if (tcsetattr(serial_port, TCSANOW, &tty) != 0)
	{
		printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
	}

	//Allocate buffer for read buffer

	
	//tcflush(serial_port, TCIFLUSH);
	
	
	return serial_port;
}

void Sensor::readSerialPort(int serialPort)
{
	// std::chrono::system_clock::time_point startGet= std::chrono::system_clock::now();
	bool notReceived=true; //we won't exit the function if not received a whole data
	while(notReceived){
		int alreadyRead = 0;
		while(true){
			alreadyRead += read(serialPort, this->serialBuf+alreadyRead, this->dataNeedRead);
			if(alreadyRead!=DATALEN){
				this->dataNeedRead = DATALEN-alreadyRead;
				// cout<<"new data need to read: "<<this->dataNeedRead<<endl;
			}
			else{
				this->dataNeedRead = DATALEN;
				
				break;
			}
		}
		// std::chrono::system_clock::time_point endGet= std::chrono::system_clock::now();
		// microsecs_t get_time(std::chrono::duration_cast<microsecs_t>(endGet-startGet));
		// cout<<"spned "<<get_time.count()<<endl;
		int curBufIdx = 0;

		//find '\n'
		
		for(int i=0;i<DATALEN;i++){
			if(this->serialBuf[i]=='\n'){
				curBufIdx++;
				
				break;
			}
			else{
				curBufIdx++;

			}
		}
		
		if(this->init){
			
			std::copy(this->serialBuf,this->serialBuf+curBufIdx,this->frontBuf_ptr);
			std::copy(this->serialBuf+curBufIdx,this->serialBuf+DATALEN,this->backBuf_ptr);
			*this->frontBuf_count+=curBufIdx;
			*this->backBuf_count+=(DATALEN-curBufIdx);
			this->backBuf_ptr +=(DATALEN-curBufIdx);
		}
		else{
			std::copy(this->serialBuf+curBufIdx,this->serialBuf+DATALEN,this->frontBuf_ptr);
			
			this->frontBuf_ptr += curBufIdx;
			*(this->frontBuf_count) +=curBufIdx;
			this->init = true;
		}
		
		if(*this->frontBuf_count == DATALEN){
			this->tempSen = this->frontBuf.get();
			
			notReceived = false;
			
		}
		


		this->frontBuf.swap(this->backBuf);
		this->frontBuf_ptr = this->backBuf_ptr;
		this->frontBuf_count.swap(this->backBuf_count);
		*this->backBuf_count=0;
		
		this->backBuf_ptr = this->backBuf.get();

		
	}
	std::chrono::system_clock::time_point curTime= std::chrono::system_clock::now();
	microsecs_t sen_time(std::chrono::duration_cast<microsecs_t>(curTime - this->origin));
	bool success = true; //due to serial port issue, sometime it will return wrong measurements, it is usually larger than 1024, if it happens, we just the old sen value for current time
	{
		std::lock_guard<std::mutex> lock(*this->senDataLock.get());
		this->senData[0] = sen_time.count();

		int idx =0;
		std::vector<int> recSenData;
		
		for (int i = 0; i < NUMSEN; i++)
		{
			int curSen = (int)(this->backBuf[idx]) + (int)(this->backBuf[idx+1] << 8);
			if(curSen<1024){
				this->senData[i + 1] = curSen;
				idx += 2;
				recSenData.push_back(this->senData[i+1]);
			}
			else{
				success = false;
				std::cout << "false sen:" << ++this->falseSenCount << std::endl;
				break;
			}

			
		
		
		}
		if(success){
			this->senRec->PushData((unsigned long)this->senData[0],recSenData);
			
		}
	}
	//the wrong measurements will still get transfer to the pc
	std::copy(this->backBuf.get(),this->backBuf.get()+DATALEN,this->senDataRaw.get());



}
void Sensor::serialPortClose(int serial_port)
{
	close(serial_port);
}

Sensor::~Sensor()
{
	// if(this->saveData_th)
	// 	this->saveData_th->join();
	std::cout << "start to delete" << std::endl;

}
