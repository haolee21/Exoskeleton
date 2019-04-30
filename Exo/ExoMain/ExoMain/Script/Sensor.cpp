#include "Sensor.h"

Sensor::Sensor(char *portName,long sampT,mutex* senLock)
{
	cout << "creating" << endl;
	if (!this->is_create) {
		
		cout << "Create Sensor" << endl;
		this->serialDevId = this->serialPortConnect(portName);
		this->sampT = sampT;
		//initialize data receiving buffer
		this->init_buffer = true; //true if we need to init
		this->senLock = senLock;
		
		//initialize the recIndex
		this->recIndex = 0;
		for(int i=0;i<recLength;i++)
			this->totSenRec[i]=new int[NUMSEN];



		if (this->serialDevId == -1)
			cout << "Sensor init failed" << endl;
	}
	else
		cout << "Sensor already created" << endl;
}

void Sensor::Start() {
	this->sw_senUpdate = true;
	memset(&this->senBuffer, '\0', sizeof(this->senBuffer));
	memset(&this->senData, 0, sizeof(this->senData));
	printf("current senBuffer: %s\n", this->senBuffer);
	this->th_SenUpdate = new thread(&Sensor::senUpdate,this);
	cout << "initial receiving thread" << endl;
	
}
void Sensor::Stop() {
	cout << "get into stop" << endl;
	this->sw_senUpdate = false;
	this->serialPortClose(this->serialDevId);
	
}
void Sensor::senUpdate() {
	while (this->sw_senUpdate) {
		std::chrono::system_clock::time_point startTime = std::chrono::system_clock::now(); //starting time
		this->readSerialPort(this->serialDevId);
		while (std::chrono::system_clock::now()-startTime
			< std::chrono::milliseconds(this->sampT)) {
			this->waitToSync(); 
		}
		typedef std::chrono::duration<int, std::milli> millisecs_t;
		std::chrono::system_clock::time_point end = std::chrono::system_clock::now();
		millisecs_t duration(std::chrono::duration_cast<millisecs_t>(end - startTime));
		cout << duration.count() << " ms \n";
	}
	cout << "sensor ends" << endl;
}


//reference from https://blog.mbedded.ninja/programming/operating-systems/linux/linux-serial-ports-using-c-cpp/

int Sensor::serialPortConnect(char *portName) {

	// Open the serial port. Change device path as needed (currently set to an standard FTDI USB-UART cable type device)
	int serial_port = open(portName, O_RDWR);

	// Create new termios struc, we call it 'tty' for convention
	struct termios tty;
	memset(&tty, 0, sizeof tty);

	// Read in existing settings, and handle any error
	if (tcgetattr(serial_port, &tty) != 0) {
		printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
	}

	tty.c_cflag &= PARENB; // set parity bit, disabling parity (most common)
	tty.c_cflag &= ~CSTOPB; // Clear stop field, only one stop bit used in communication (most common)
	tty.c_cflag |= CS8; // 8 bits per byte (most common)
	tty.c_cflag &= ~CRTSCTS; // Disable RTS/CTS hardware flow control (most common)
	tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)

	tty.c_lflag &= ~ICANON;
	tty.c_lflag &= ~ECHO; // Disable echo
	tty.c_lflag &= ~ECHOE; // Disable erasure
	tty.c_lflag &= ~ECHONL; // Disable new-line echo
	tty.c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP
	tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
	tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL); // Disable any special handling of received bytes

	tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
	tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed
	// tty.c_oflag &= ~OXTABS; // Prevent conversion of tabs to spaces (NOT PRESENT ON LINUX)
	// tty.c_oflag &= ~ONOEOT; // Prevent removal of C-d chars (0x004) in output (NOT PRESENT ON LINUX)

	tty.c_cc[VTIME] = 0;    // Wait for up to 1s (10 deciseconds), returning as soon as any data is received.
	tty.c_cc[VMIN] = 0;

	// Set in/out baud rate to be 115200
	cfsetispeed(&tty, B1000000);
	cfsetospeed(&tty, B1000000);

	// Save tty settings, also checking for error
	if (tcsetattr(serial_port, TCSANOW, &tty) != 0) {
		printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
	}

	//Allocate buffer for read buffer


	memset(&this->senBuffer, '\0', sizeof(this->senBuffer));

	// Read bytes. The behaviour of read() (e.g. does it block?,
	// how long does it block for?) depends on the configuration
	// settings above, specifically VMIN and VTIME
	int num_bytes = read(serial_port, &this->senBuffer, sizeof(this->senBuffer));

	// n is the number of bytes read. n may be 0 if no bytes were received, and can also be -1 to signal an error.
	if (num_bytes < 0) {
		printf("Error reading: %s", strerror(errno));
	}

	// Here we assume we received ASCII data, but you might be sending raw bytes (in that case, don't try and
	// print it to the screen like this!)

	return serial_port;

}

void Sensor::readSerialPort(int serialPort) {
	// This function reads signals, put into buffer, and update the senData once it complete the data receiving
	bool getFullData = false; //each time we must retrieve a full data
	bool foundHead = false;
	
	int *getChar = this->tempSen;
	while (!getFullData) {
		char *currentRead = this->senBuffer;
		int n_bytes = read(serialPort, &this->senBuffer, sizeof(senBuffer));//the last one is end
		//clear the initial data in raspberry pi's serial port, they do not make sense
		if (this->init_buffer) {
			this->init_buffer = false;
			memset(&this->senBuffer, '\0', sizeof(this->senBuffer));
			continue;
		}
		if (n_bytes < 0)
			continue;
		//first find the beginning of current data
		for (int i = 0; i < n_bytes; i++) {
			if (!foundHead) {
				if (*currentRead == '@') {
					foundHead = true;
				}
				currentRead++;
			}
			else {
				if (*currentRead != '\n') {
					// We will get ascii in we directly use (int)currentRead
					// However, the relative distance of it wrt to '0' is the number
					*getChar = (int)(*currentRead-'0');
					getChar++;
				}
				else {
					getFullData = true;
					break;
				}
				currentRead++;
			}
		}
		
	}
	// The measurements transform into array and prints
	int k = 0;
	std::lock_guard<std::mutex> lock(*this->senLock);
	//cout << "get data: ";
	for (int t = 0; t < NUMSEN; t++) {
		int val = 0;
		//For each measurement, the data is 
		for (int i = 0; i < this->dataFormat[t]; i++) {
			val = val * 10 + this->tempSen[k];
			k++;
		}
		// put sense data into array
		//cout << val << ',';
		this->senData[t] = val;
		// record sense data 
		this->totSenRec[this->recIndex][t]=val;
	}
	
	this->recIndex ++;
	
	//cout << endl;
	
}
void Sensor::serialPortClose(int serial_port) {
	close(serial_port);
}

void Sensor::waitToSync() {
	struct timespec ts = { 0 };
	ts.tv_sec = 0;
	ts.tv_nsec = 10000L; //10 us
	nanosleep(&ts, (struct timespec*)NULL);
}
Sensor::~Sensor()
{
	std::cout<<"start to delete"<<std::endl;
	for(int i=0;i<this->recIndex;i++){
		delete[] this->totSenRec[i];
	}
	delete[] this->totSenRec;
}