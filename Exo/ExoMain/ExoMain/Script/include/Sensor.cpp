#include "Sensor.h"
#include <memory>

#define MY_PRIORITY (49)             /* we use 49 as the PRREMPT_RT use 50 \
                                        as the priority of kernel tasklets \
                                        and interrupt handler by default */
#define POOLSIZE (200 * 1024 * 1024) // 200MB
#define MAX_SAFE_STACK (100 * 1024)  /* The maximum stack size which is \
                                      guranteed safe to access without  \
                                      faulting */

#define NSEC_PER_SEC (1000000000) // The number of nsecs per sec.

#define NSEC 1
#define USEC (1000 * NSEC)
#define MSEC (1000 * USEC)
#define SEC (1000 * MSEC)




typedef std::chrono::duration<long, std::nano> nanosecs_t;
typedef std::chrono::duration<int, std::micro> microsecs_t;
typedef std::chrono::duration<unsigned long, std::micro> microsecs_ul;
typedef std::chrono::duration<int, std::milli> millisecs_t;
Sensor::Sensor(std::string _filePath,char *portName, long sampT,Com *_com,bool _display)
{
	this->display = _display;
	std::cout << "creating" << endl;
	if (!this->is_create)
	{
		this->filePath = _filePath;
		std::cout << "Create Sensor" << endl;
		this->serialDevId = this->serialPortConnect(portName);
		this->sampT = sampT;
		
		//initialize data receiving buffer
		this->curBufIndex = 0;
		this->dataCollect = 0;
		this->noHead = true;
		this->curHead = this->curBuf;

		if (this->serialDevId == -1)
			std::cout << "Sensor init failed" << endl;

		this->curBuf = this->senBuffer;
		this->curHead = this->curBuf;
		this->senRec = new Recorder<int>("sen",_filePath,"time,sen1,sen2,sen3,sen4,sen5,sen6,sen7,sen8,sen9");
		
		//Controller
		this->com = _com;		
	}
	else
		std::cout << "Sensor already created" << endl;
}

void Sensor::Start(std::chrono::system_clock::time_point startTime)
{
	this->origin = startTime;
	this->sw_senUpdate = true;
	memset(&this->senBuffer, '\0', SIZEOFBUFFER);
	memset(&this->senData, 0, DATALEN + 1);
	//printf("current senBuffer: %s\n", this->senBuffer);
	
	this->th_SenUpdate = new thread(&Sensor::senUpdate, this);
	
	std::cout << "initial receiving thread" << endl;
	
}
void Sensor::Stop()
{
	std::cout << "get into stop" << endl;
	this->sw_senUpdate = false;
	this->serialPortClose(this->serialDevId);
	this->th_SenUpdate->join();
	std::cout<<"sensor fully stops\n";
	
}
//timer
void Sensor::tsnorm(struct timespec *ts)
{
    while (ts->tv_nsec >= NSEC_PER_SEC)
    {
        ts->tv_nsec -= NSEC_PER_SEC;
        ts->tv_sec++;
    }
}
//
void Sensor::senUpdate()
{
	//for accurate timer
	struct timespec t;
    //struct sched_param param;
    long int interval = this->sampT*USEC;
    // long int cnt = 0;
    // long int cnt1 = 0;
	clock_gettime(CLOCK_MONOTONIC, &t);
	t.tv_nsec += 0 * MSEC;
    this->tsnorm(&t);
	//
	Controller con = Controller(this->filePath,this->com,this->display);
	

	
	int conLoopCount = 1;
	std::unique_ptr<std::thread> conTh;
	bool conStart = false;

	while (this->sw_senUpdate)
	{	
		//timer
		clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t, NULL);
		//

		this->readSerialPort(this->serialDevId);
		if(conLoopCount++ ==4){
			conLoopCount = 0;
			if(conStart) 
				(*conTh).join();
			else
				conStart = true;
			conTh.reset(new std::thread(&Controller::ConMainLoop,&con,this->senData));
		
			//std::cout<<"data len= "<<sizeof(con.GetValCond());
			//disp.send(&((char)con.GetValCond()),sizeof(con.GetValCond()))
			
		}
		
		// timer
		// calculate next shot
        t.tv_nsec += interval;
        this->tsnorm(&t);


	}
	(*conTh).join();
	std::cout << "sensor ends" << endl;
	
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

	tty.c_cflag &= PARENB;		   // set parity bit, disabling parity (most common)
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

	tty.c_cc[VTIME] = 0.01; // Wait for up to 1s (10 deciseconds), returning as soon as any data is received.
	tty.c_cc[VMIN] = 24;

	// Set in/out baud rate to be 115200
	cfsetispeed(&tty, B1000000);
	cfsetospeed(&tty, B1000000);

	// Save tty settings, also checking for error
	if (tcsetattr(serial_port, TCSANOW, &tty) != 0)
	{
		printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
	}

	//Allocate buffer for read buffer

	memset(&this->senBuffer, '\0', DATALEN);
	tcflush(serial_port, TCIFLUSH); 

	return serial_port;
}

void Sensor::readSerialPort(int serialPort)
{
	//Collect data from the serial port's buffer
	//Basic operation: 
	// 1. Check if we collect enough data (data collect > DATALEN)
	// 2. Find the head of it
	// 3. Check if *(head+DATALEN-1) == '\n', if false we skipped the number of bytes we received, and find the head again
	// 4. Conver the data back to numbers

	int n_bytes = read(serialPort, this->curBuf, DATALEN);
	
	if(n_bytes==DATALEN){
		//std::cout<<"got data "<<this->curBufIndex<<std::endl;;
		this->dataCollect+= n_bytes;
		this->curBuf+=DATALEN; // we advanced to next buffer as long as we received DATALEN bytes
		this->curBufIndex+=DATALEN;

		//std::cout<<"data collect: "<<this->dataCollect<<std::endl;
		if(this->curBufIndex>=(SIZEOFBUFFER-1)){ // goes back to the beginning of the buffer when we reaches the end
			//std::cout<<"end of buffer "<<this->curBufIndex<<std::endl;
			this->curBuf = this->senBuffer;
			this->curBufIndex=0;
			
		}

		// Find the beginning of the data '@'
		if(this->noHead){
			// this->curHead = this->curBuf-DATALEN; //if we have not found the head, we started from the newest data
			
			for(int i=0;i<n_bytes;i++){
				this->dataCollect--;
				if(*this->curHead=='@'){
					this->noHead = false;
					break;
				}
				else{
					this->curHead++;
				}
			}
		}
		else{//if we already found the head
			if((this->dataCollect)>=DATALEN){ //we only check the tail if we have enough data
				char tempSenData[DATALEN]; //temperary transformed data, including the '@'
				// we need to do this in seperate loop is because of curHead
				// it is possible that the curHead is pointing at the end of the buffer, which need to be re-direct to the beginning of the pointer
				// yet, this need to be handle independent from dataCollect or curBuf (it is about we read until which data, not which we collected)
				
				for(int i=0;i<DATALEN;i++){
					tempSenData[i] = *(this->curHead); //take out the last data
					if((this->curHead)==(this->senBuffer+SIZEOFBUFFER-1)){
						this->curHead = this->senBuffer; //point it back to the beginning of the buffer if it is the last one
					}	
					else
					{
						this->curHead++;
					}		
				}
				if(tempSenData[DATALEN-1]=='\n'){
					std::chrono::system_clock::time_point curTime= std::chrono::system_clock::now();
					microsecs_t sen_time(std::chrono::duration_cast<microsecs_t>(curTime - this->origin));
					int timeNow = sen_time.count(); 
					{
						
						this->senData[0] = timeNow;
						this->senData[1] = (int)(tempSenData[1]) + (int)(tempSenData[2] << 8);
						this->senData[2] = (int)(tempSenData[3]) + (int)(tempSenData[4] << 8);
						this->senData[3] = (int)(tempSenData[5]) + (int)(tempSenData[6] << 8);
						this->senData[4] = (int)(tempSenData[7]) + (int)(tempSenData[8] << 8);
						this->senData[5] = (int)(tempSenData[9]) + (int)(tempSenData[10] << 8);
						this->senData[6] = (int)(tempSenData[11]) + (int)(tempSenData[12] << 8);
						this->senData[7] = (int)(tempSenData[13]) + (int)(tempSenData[14] << 8);
						this->senData[8] = (int)(tempSenData[15]) + (int)(tempSenData[16] << 8);
						this->senData[9] = (int)(tempSenData[17]) + (int)(tempSenData[18] << 8);
					}
					std::vector<int> recSenData;
					for(int i=1;i<NUMSEN+1;i++){
						recSenData.push_back(senData[i]);
					}
					this->senRec->PushData((unsigned long)this->senData[0],recSenData);
					// std::cout<<"read ";
					// for(int i=0;i<DATALEN;i++)
					// 	std::cout<<tempSenData[i];
					
					
					// for some reason, I cannot just let noHead = true after I find the data
					// Also, we shouldn't search for the head again since we know the next byte is head
					if(*(this->curHead)!='@'){
						//std::cout<<"next is not head "<<*this->curHead<<std::endl;
						this->noHead = true;
						this->dataCollect = 0;
					}
					this->dataCollect -=DATALEN;
				}
				else{
					// this is not possible to have enough data but no tail, directly redo everything
					//std::cout<<"no tail\n\n\n";
					this->curBuf = this->senBuffer;
					this->dataCollect =0;
					this->curBufIndex =0;
					this->noHead = true;
					this->curHead = this->curBuf;
				}
			
			}

		}

	}
	else{// if we cannot get correct number of data, we should clear the buffer since there is no point
		this->curBuf = this->senBuffer;
		this->dataCollect =0;
		this->curBufIndex =0;
		this->noHead = true;
		this->curHead = this->curBuf;
	
	}

	// this->recIndex++;
}
void Sensor::serialPortClose(int serial_port)
{
	close(serial_port);
}

Sensor::~Sensor()
{
	std::cout << "start to delete" << std::endl;
	

	delete this->senRec;
	delete this->th_SenUpdate;

}