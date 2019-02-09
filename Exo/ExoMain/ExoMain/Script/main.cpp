
#include<iostream>
#include<cstdio>
#include<thread>
#include<mutex>
#include "Sensor.h"
#include<unistd.h> //for linux sleep
#include "PWM.h"
mutex SenLock;
void DelaySys(int waitTime) {
	struct timespec ts2 = { 0 };
	ts2.tv_sec = waitTime;
	ts2.tv_nsec = 10000L; //10 us
	nanosleep(&ts2, (struct timespec*)NULL);
}

using namespace std;
int main(void)
{
	wiringPiSetupSys(); //setup the system ports, timer, etc. 
	char portName[] = "/dev/ttyACM0";
	Sensor sensor = Sensor(portName, 5L,&SenLock);
	sensor.Start();
	cout << "finish creating" << endl;
	DelaySys(10);
	
	sensor.Stop();
	
	
	return 0;
}
