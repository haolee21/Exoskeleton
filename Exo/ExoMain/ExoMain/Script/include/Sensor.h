#ifndef SENSOR_H
#define SENSOR_H
#include "Controller.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sched.h>
#include <sys/mman.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <iostream>
#include <fcntl.h>
#include <sched.h>
#include <sys/io.h>
#include <string.h>
#include <signal.h>
#include <vector>
#include <malloc.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>

#include <sys/ioctl.h>

#include <thread>
#include<iostream>
#include <wiringSerial.h>

#include <wiringPi.h>
#include <chrono>
#include<ctime> //this timer

#include <stdio.h>
#include <cstring>
#include <fcntl.h> // Contains file controls like O_RDWR
#include <errno.h> // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h> // write(), read(), close()
#include <mutex>

//data recording
#include "Recorder.hpp"
const int NUMSEN = 9;
const int DATALEN =NUMSEN*2+2;
const int SIZEOFBUFFER= DATALEN*1000;


using namespace std;

const int recLength = 240000; //This is the pre-allocate memory for recording sensed data

class Sensor
{
public:
	
	Sensor(char *port,long sampTmilli,mutex* senLock); //sampT is in milli
	~Sensor();
	
	void Start(std::chrono::system_clock::time_point startTime);
	void Stop();
	int senData[NUMSEN+1]; //data get from ADC
	thread *th_SenUpdate;

	//create array to storage sensing data
	int **totSenRec = new int*[recLength];
	
	int recIndex;
private:
	std::chrono::system_clock::time_point origin;
	bool is_create = false;
	int serialDevId;
	bool sw_senUpdate;
	void senUpdate();
	long sampT;
	
	//variables for receiving data

	char senBuffer[SIZEOFBUFFER];
	char tempSen[DATALEN];
	char *curHead;
	char *curBuf;
	int curBufIndex;
	bool noHead;
	int dataCollect;



	mutex* senLock;
	
	int preTime;
	int serialPortConnect(char *portName);
	void readSerialPort(int serialPort);
	void serialPortClose(int serial_port);
	void waitToSync(std::chrono::system_clock::time_point,long extraWait);
	
	//functions that Ji used
	
	void tsnorm(struct timespec *ts);

	//Controller
	
	std::thread *conTh;
	void callCon();
	//for data recording
	Recorder *senRec;
};



#endif //SENSOR_H