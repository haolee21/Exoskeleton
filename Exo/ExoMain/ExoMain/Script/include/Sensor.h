#ifndef SENSOR_H
#define SENSOR_H
#include "common.hpp"
#include "Controller.h"
#include "BWFilter.hpp"
#include "SenBuffer.hpp"
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
#include <signal.h>
#include <vector>
#include <malloc.h>


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
#include "Encoder.hpp"


const int recLength = 240000; //This is the pre-allocate memory for recording sensed data


#define MY_STACK_SIZE       (100*1024)      /* 100 kB is enough for now. */

class Sensor
{
public:
	
	Sensor(string _filePath,char *port,long sampTmicro,std::shared_ptr<Com> com,bool display); //sampT is in milli
	~Sensor();
	
	void Start(std::chrono::system_clock::time_point startTime);
	void Stop();
	
	
	std::shared_ptr<int[]> oriData; //the size of array seems important when ~Sensor() is called
	std::shared_ptr<int[]> senData;
	std::shared_ptr<char[]> senDataRaw;
	int tempSenData[NUMSEN];//this is for checking the temp senData after we read it from arduino, if any of the senData>1024, all senData is waived
	//std::shared_ptr<std::mutex> senDataLock;
	std::mutex *senDataLock;
	pthread_t th_SenUpdate;
	pthread_attr_t attr;

	// thread *th_SenUpdate;

	
private:
	std::unique_ptr<Encoder> LHip_s;
	std::unique_ptr<Encoder> LHip_f;
	std::unique_ptr<Encoder> RHip_s;
	std::unique_ptr<Encoder> RHip_f;
	std::unique_ptr<Encoder> LKne_s;
	std::unique_ptr<Encoder> RKne_s;
	std::unique_ptr<Encoder> LAnk_s;
	std::unique_ptr<Encoder> RAnk_s;

	std::chrono::system_clock::time_point origin;
	bool sw_senUpdate;
	
	
	static void *senUpdate(void *sen);
	long sampT;
	std::mutex senUpdateLock;
	//variables for receiving data

	char serialBuf[SIZEOFBUFFER];
	char senTempBuf[SIZEOFBUFFER];
	SenBuffer senBuff;
	char outBuff[DATALEN];


	char *tempSen;
	bool init;

	int dataNeedRead = DATALEN;
	int falseSenCount;

	bool senNotInit = true;


	

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
	
	shared_ptr<Recorder<int,16>> senRec; //smart pointer test, failed, don't know why since it works in simpler cases

	unique_ptr<Recorder<int,1>> SampTimeRec;
};



#endif //SENSOR_H