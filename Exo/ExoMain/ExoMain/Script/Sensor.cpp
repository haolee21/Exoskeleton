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

		cout << senData << endl;
		cout << "Create Sensor" << endl;
		this->serialDevId = this->serialPortConnect(portName);
		this->sampT = sampT;
		//initialize data receiving buffer
		this->init_buffer = true; //true if we need to init
		this->senLock = senLock;
		this->curBufIndex = 0;

		this->preTime = 0; //for testing purpose

		// //initialize the recIndex
		// this->recIndex = 0;
		// for (int i = 0; i < recLength; i++)
		// 	this->totSenRec[i] = new int[NUMSEN];

		if (this->serialDevId == -1)
			cout << "Sensor init failed" << endl;

		this->curBuf = this->senBuffer;
		this->curHead = this->curBuf;
		this->init_buffer = false;
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

	serialFlush(this->serialDevId);
	t.tv_nsec += 0 * MSEC;
    this->tsnorm(&t);
	while (this->sw_senUpdate)
	{
		clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t, NULL);
		//startTime = std::chrono::system_clock::now();
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
				std::cout << "we wait for " << t_duration.count() << std::endl;
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
		}
		catch (std::logic_error &)
		{
			std::cout<< "[exception caught]\n";
		}
		this->preTime = this->senData[0];


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

	tty.c_cc[VTIME] = 0; // Wait for up to 1s (10 deciseconds), returning as soon as any data is received.
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

	memset(&this->senBuffer, '\0', sizeof(this->senBuffer));

	// Read bytes. The behaviour of read() (e.g. does it block?,
	// how long does it block for?) depends on the configuration
	// settings above, specifically VMIN and VTIME

	int num_bytes = read(serial_port, &this->senBuffer, sizeof(this->senBuffer));

	// n is the number of bytes read. n may be 0 if no bytes were received, and can also be -1 to signal an error.
	if (num_bytes < 0)
	{
		printf("Error reading: %s", strerror(errno));
	}

	// Here we assume we received ASCII data, but you might be sending raw bytes (in that case, don't try and
	// print it to the screen like this!)

	return serial_port;
}

void Sensor::readSerialPort(int serialPort)
{

	// This function reads signals, put into buffer, and update the senData once it complete the data receiving
	bool getFullData = false; //each time we must retrieve a full data
	bool notGetHead = true;
	if (!this->init_buffer)
	{
		this->curHead = this->curBuf;
		int n_bytes = read(serialPort, this->curBuf, DATALEN);
		this->curBuf += DATALEN;
		this->curBufIndex += DATALEN;
		this->init_buffer = true;
	}

	// make sure there are enough memory for buffer
	if ((this->curBufIndex + DATALEN) >= SIZEOFBUFFER)
	{

		this->curBuf = this->senBuffer;
		this->curBufIndex = 0;
		this->curHead = this->curBuf;

		read(serialPort, this->curBuf, DATALEN);
		//std::cout<<"preload\n";
		this->curBuf += DATALEN;
		this->curBufIndex += DATALEN;
	}

	//std::cout<<static_cast<const void*> (this->curBuf)<<std::endl; //how you print the char array's address
	//std::chrono::system_clock::time_point start_time = std::chrono::system_clock::now();
	int n_bytes = read(serialPort, this->curBuf, DATALEN);
	//std::chrono::system_clock::time_point end_time = std::chrono::system_clock::now();
	//microsecs_t t_duration(std::chrono::duration_cast<microsecs_t>(end_time - start_time));
	//std::cout << "we wait for " << t_duration.count() << std::endl;

	//after received data, curBuf and curBufIndex all advanced
	this->curBuf += DATALEN;
	this->curBufIndex += DATALEN;
	//std::chrono::system_clock::time_point start_time = std::chrono::system_clock::now();
	while (notGetHead)
	{
		if (*this->curHead == '@')
		{
			//std::cout<<"found head\n";
			notGetHead = false;
		}
		else
		{
			this->curHead++;
		}
	}
	if (*(this->curHead + DATALEN - 1) == '\n')
	{
		//std::cout<<"we got tail\n";
		this->curHead++;
		getFullData = true;
	}
	else
	{
		notGetHead = true;
		this->curHead += 1;
	}
	if (getFullData && this->sw_senUpdate)
	{

		std::lock_guard<std::mutex> lock(*this->senLock);
		this->senData[0] = (int)(((unsigned long)this->curHead[0]) + ((unsigned long)(this->curHead[1] << 8)) + ((unsigned long)(this->curHead[2] << 16)) + ((unsigned long)(this->curHead[3] << 24)));
		this->senData[1] = (int)(this->curHead[4]) + (int)(this->curHead[5] << 8);
		this->senData[2] = (int)(this->curHead[6]) + (int)(this->curHead[7] << 8);
		this->senData[3] = (int)(this->curHead[8]) + (int)(this->curHead[9] << 8);
		this->senData[4] = (int)(this->curHead[10]) + (int)(this->curHead[11] << 8);
		this->senData[5] = (int)(this->curHead[12]) + (int)(this->curHead[13] << 8);
		this->senData[6] = (int)(this->curHead[14]) + (int)(this->curHead[15] << 8);
		this->senData[7] = (int)(this->curHead[16]) + (int)(this->curHead[17] << 8);
		this->senData[8] = (int)(this->curHead[18]) + (int)(this->curHead[19] << 8);
		this->senData[9] = (int)(this->curHead[20]) + (int)(this->curHead[21] << 8);

		//std::cout<<this->senData[0]-this->preTime<<std::endl;  //this is for checking the sensing time gap is correct
		//this->preTime = this->senData[0];
		// cout << "get data: ";
		// cout << this->senData[0] << ',' << this->senData[1] << ',' << this->senData[2] << ',' << this->senData[3] << ',';
		// cout << this->senData[4] << ',' << this->senData[5] << ',' << this->senData[6] << ',' << this->senData[7] << ',';
		// cout << this->senData[8] << ',' << this->senData[9] << std::endl;
	}

	// // // transform time data

	// this->recIndex++;
}
void Sensor::serialPortClose(int serial_port)
{
	close(serial_port);
}

void Sensor::waitToSync(std::chrono::system_clock::time_point startTime, long extraWait)
{
	if (extraWait < 0)
	{
		extraWait = 0L;
	}
	else
	{
		extraWait += 1000L;
	}
	std::chrono::system_clock::time_point nowTime = std::chrono::system_clock::now();
	nanosecs_t t_duration(std::chrono::duration_cast<nanosecs_t>(nowTime - startTime));
	long waitTime = (this->sampT - t_duration.count() - extraWait);
	struct timespec ts = {0};
	ts.tv_sec = 0;
	ts.tv_nsec = waitTime;
	nanosleep(&ts, (struct timespec *)NULL);
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