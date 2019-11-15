#ifndef SENSOR_H
#define SENSOR_H
#include "common.hpp"
#include "Controller.h"
#include "BWFilter.hpp"

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

#include <chrono>
#include<ctime> //this timer

#include <stdio.h>
#include <cstring>
#include <fcntl.h> // Contains file controls like O_RDWR
#include <errno.h> // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h> // write(), read(), close()
#include <mutex>
#include <memory>
#include<pthread.h>
//data recording
#include "Recorder.hpp"
//need to sync with controller's

using namespace std;

const int recLength = 240000; //This is the pre-allocate memory for recording sensed data


#define MY_STACK_SIZE       (100*1024)      /* 100 kB is enough for now. */

class Sensor
{
public:
	
	Sensor(string _filePath,char *port,long sampTmicro,Com *com,bool display); //sampT is in milli
	~Sensor();
	
	void Start(std::chrono::system_clock::time_point startTime);
	void Stop();
	
	int oriData[NUMSEN]; //original data
	int senData[NUMSEN+1]; //data get from ADC after filter
	


	char senDataRaw[DATALEN];
	std::shared_ptr<std::mutex> senDataLock;

	pthread_t th_SenUpdate;
	pthread_attr_t attr;

	// thread *th_SenUpdate;

	
private:
	std::chrono::system_clock::time_point origin;
	bool is_create = false;
	int serialDevId;
	bool sw_senUpdate;
	static void* senUpdate(void* sen);
	long sampT;
	
	//variables for receiving data

	// char senBuffer[SIZEOFBUFFER];
	// char tempSen[DATALEN];
	// char *curHead;
	// char *curBuf;
	// int curBufIndex;
	// bool noHead;
	// int dataCollect;

	char serialBuf[SIZEOFBUFFER];
	
	std::shared_ptr<int> frontBuf_count;
	std::shared_ptr<int> backBuf_count;
	std::shared_ptr<char[]>frontBuf;
	std::shared_ptr<char[]>backBuf;
	char *frontBuf_ptr;
	char *backBuf_ptr;

	char *tempSen;
	bool init;

	int dataNeedRead = DATALEN;



	bool senNotInit = true;


	
	
	int serialPortConnect(char *portName);
	void readSerialPort(int serialPort);
	void serialPortClose(int serial_port);
	
	//Lowpass butterworth filter, this can be implented to arduino if we replace arduino mega with better MCU chips
	//BWFilter bFilter;
	bool filterInit_flag = false;


	
	//functions that Ji used
	
	// void tsnorm(struct timespec *ts);

	//Displayer
	bool display;
	//Controller
	
	Com *com;
	std::thread *conTh;
	void callCon();
	//for data recording

	std::string filePath;
	std::shared_ptr<std::thread> saveData_th;
	void SaveAllData();
	
	shared_ptr<Recorder<int>> senRec; //smart pointer test, failed, don't know why since it works in simpler cases
	
	
};



#endif //SENSOR_H