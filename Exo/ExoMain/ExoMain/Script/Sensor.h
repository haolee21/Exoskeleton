#ifndef SENSOR_H
#define SENSOR_H

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
using namespace std;
const int NUMSEN = 9; //numSen
const int SIZEOFBUFFER = 512; //Watchout this value, I use this buffer twice to catch data, it has to be long enough
const int BUFFERSWITCH = 150;
const int DATALEN = NUMSEN*2+2;

const int recLength = 240000; //This is the pre-allocate memory for recording sensed data

class Sensor
{
public:
	Sensor(char *port,long sampTmilli,mutex* senLock); //sampT is in millisecond
	~Sensor();
	
	void Start();
	void Stop();
	int senData[NUMSEN+1]; //data get from ADC
	thread *th_SenUpdate;

	//create array to storage sensing data
	int **totSenRec = new int*[recLength];
	
	int recIndex;
private:
	
	bool is_create = false;
	int serialDevId;
	bool sw_senUpdate;
	void senUpdate();
	long sampT;
	
	//variables for receiving data
	bool init_buffer;
	char senBuffer[SIZEOFBUFFER];
	char tempSen[DATALEN];
	mutex* senLock;
	
	

	int dataFormat[9] = {7,4,4,4,4,4,4,4,4};	//indicate how many digits of measurements

	int serialPortConnect(char *portName);
	void readSerialPort(int serialPort);
	void serialPortClose(int serial_port);
	void waitToSync();
};



#endif // !SENSOR_H