#include "Sensor.h"
#include "Controller.h"

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

long int ms_cnt = 0;


typedef std::chrono::duration<long, std::nano> nanosecs_t;
typedef std::chrono::duration<int, std::micro> microsecs_t;
typedef std::chrono::duration<int, std::milli> millisecs_t;
Sensor::Sensor(char *portName, long sampT, mutex *senLock)
{
	cout << "creating" << endl;
	if (!this->is_create)
	{
		cout << "Create Sensor" << endl;
		this->serialDevId = this->serialPortConnect(portName);
		this->sampT = sampT;
		
		//initialize data receiving buffer
		
		this->senLock = senLock;
		this->curBufIndex = 0;
		this->preTime = 0; //for testing purpose

		this->dataCollect = 0;
		this->noHead = true;
		this->curHead = this->curBuf;
	
		// //initialize the recIndex
		// this->recIndex = 0;
		// for (int i = 0; i < recLength; i++)
		// 	this->totSenRec[i] = new int[NUMSEN];

		if (this->serialDevId == -1)
			cout << "Sensor init failed" << endl;

		this->curBuf = this->senBuffer;
		this->curHead = this->curBuf;
		
	}
	else
		cout << "Sensor already created" << endl;
}

void Sensor::Start()
{
	this->sw_senUpdate = true;
	memset(&this->senBuffer, '\0', SIZEOFBUFFER);
	memset(&this->senData, 0, DATALEN + 1);
	//printf("current senBuffer: %s\n", this->senBuffer);
	this->th_SenUpdate = new thread(&Sensor::senUpdate, this);
	cout << "initial receiving thread" << endl;
}
void Sensor::Stop()
{
	cout << "get into stop" << endl;
	this->sw_senUpdate = false;
	this->serialPortClose(this->serialDevId);

	//wait for 1 sec to avoid segmentation error
	struct timespec ts2 = {0};
	ts2.tv_sec = 1;
	ts2.tv_nsec = 0; //10 us
	nanosleep(&ts2, (struct timespec *)NULL);
}

void Sensor::tsnorm(struct timespec *ts)
{
    while (ts->tv_nsec >= NSEC_PER_SEC)
    {
        ts->tv_nsec -= NSEC_PER_SEC;
        ts->tv_sec++;
    }
}
void Sensor::senUpdate()
{
	long extraWait = 0L;
	std::chrono::system_clock::time_point startTime = std::chrono::system_clock::now();
	int conLoopCount = 0;
	Controller con = Controller(startTime);
	bool testSen = true;
	bool testState = true;
	std::chrono::system_clock::time_point sendTest;
	std::chrono::system_clock::time_point senseTest;
	

	struct timespec t;
    struct sched_param param;
    long int interval = this->sampT*USEC;
    long int cnt = 0;
    long int cnt1 = 0;

	clock_gettime(CLOCK_MONOTONIC, &t);

	
	t.tv_nsec += 0 * MSEC;
    this->tsnorm(&t);
	
	while (this->sw_senUpdate)
	{
		clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t, NULL);
		this->readSerialPort(this->serialDevId);
		if (conLoopCount++ == 1)
		{
			conLoopCount = 0;
			if (testSen)
			{
				testSen = false;
				sendTest = std::chrono::system_clock::now();
				con.SendTestMeasurement(testState);
				//std::cout << "trigger\n";
			}
		}
		else
		{
			if (con.WaitTestMeasurement(senseTest, testState, this->senData))
			{
				testSen = true;
				millisecs_t t_duration(std::chrono::duration_cast<millisecs_t>(senseTest - sendTest));
				//std::cout << "we wait for " << t_duration.count() << std::endl;
			}
		}
		
		std::chrono::system_clock::time_point endReadTime = std::chrono::system_clock::now();
		nanosecs_t t_duration1(std::chrono::duration_cast<nanosecs_t>(endReadTime - startTime));
		startTime = endReadTime;
		
		//std::cout << "real " << t_duration1.count() << std::endl;
		try
		{
			std::lock_guard<std::mutex> lock(*this->senLock);
			//std::cout << this->senData[0] - this->preTime << std::endl;
			this->preTime = this->senData[0];
		}
		catch (std::logic_error &)
		{
			std::cout<< "[exception caught]\n";
		}
		
		// calculate next shot
        t.tv_nsec += interval;

        this->tsnorm(&t);

        if (cnt++ >= 1000)
        {
            //printf("%ld\n", cnt1);
            cnt = 1;
        }
        ms_cnt++;
        cnt1++;


	}
	cout << "sensor ends" << endl;
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
	// std::cout<<"buff "<<this->curBuf[0]<<this->curBuf[1]<<this->curBuf[2]<<this->curBuf[3]
	// <<this->curBuf[4]<<this->curBuf[5]<<this->curBuf[6]<<this->curBuf[7]<<this->curBuf[8]
	// <<this->curBuf[9]<<this->curBuf[10]<<this->curBuf[11]<<this->curBuf[12]<<this->curBuf[13]
	// <<this->curBuf[14]<<this->curBuf[15]<<this->curBuf[16]<<this->curBuf[17]<<this->curBuf[18]
	// <<this->curBuf[19]<<this->curBuf[20]<<this->curBuf[21]<<this->curBuf[22]<<this->curBuf[23]<<std::endl;
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
					
					std::lock_guard<std::mutex> lock(*this->senLock);
					this->senData[0] = (int)(((unsigned long)tempSenData[1]) + ((unsigned long)(tempSenData[2] << 8)) + ((unsigned long)(tempSenData[3] << 16)) + ((unsigned long)(tempSenData[4] << 24)));
					this->senData[1] = (int)(tempSenData[5]) + (int)(tempSenData[6] << 8);
					this->senData[2] = (int)(tempSenData[7]) + (int)(tempSenData[8] << 8);
					this->senData[3] = (int)(tempSenData[9]) + (int)(tempSenData[10] << 8);
					this->senData[4] = (int)(tempSenData[11]) + (int)(tempSenData[12] << 8);
					this->senData[5] = (int)(tempSenData[13]) + (int)(tempSenData[14] << 8);
					this->senData[6] = (int)(tempSenData[15]) + (int)(tempSenData[16] << 8);
					this->senData[7] = (int)(tempSenData[17]) + (int)(tempSenData[18] << 8);
					this->senData[8] = (int)(tempSenData[19]) + (int)(tempSenData[20] << 8);
					this->senData[9] = (int)(tempSenData[21]) + (int)(tempSenData[22] << 8);

					// std::cout<<"read ";
					// for(int i=0;i<DATALEN;i++)
					// 	std::cout<<tempSenData[i];
					
					
					// for some reason, I cannot just let noHead = true after I find the data
					// Also, we shouldn't search for the head again since we know the next byte is head
					if(*(this->curHead)!='@'){
						std::cout<<"next is not head "<<*this->curHead<<std::endl;
						this->noHead = true;
						this->dataCollect = 0;
					}
					this->dataCollect -=DATALEN;
				}
				else{
					// this is not possible to have enough data but no tail, directly redo everything
					std::cout<<"no tail\n\n\n";
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
	// for (int i = 0; i < this->recIndex; i++)
	// {
	// 	delete[] this->totSenRec[i];
	// }
	// delete[] this->totSenRec;
}