
#include<iostream>
#include<cstdio>
#include<thread>
#include<mutex>
#include "Sensor.h"
#include<unistd.h> //for linux sleep
#include "PWM.h"
#include "Controller.h"
mutex SenLock;
void DelaySys(int waitTime) {
	struct timespec ts2 = { 0 };
	ts2.tv_sec = waitTime;
	ts2.tv_nsec = 10000L; //10 us
	nanosleep(&ts2, (struct timespec*)NULL);
}
void ReadSenData(Sensor *sensor){
	std::lock_guard<std::mutex> lock(SenLock);
	for (int i=0;i<10;i++){
		std::cout<<sensor->senData[i];
		std::cout<<",";
	}
	std::cout<<endl;
}

using namespace std;
int main(void)
{
	wiringPiSetupSys(); //setup the system ports, timer, etc. 
	char portName[] = "/dev/ttyACM0";
	Sensor sensor = Sensor(portName, 5L,&SenLock);
	sensor.Start();
	cout << "finish creating" << endl;
	Controller con = Controller();
	con.TestValve();
	/*
	for(int i=0;i<40;i++){
		ReadSenData(&sensor);
		DelaySys(1);
	}*/
	
	
	sensor.Stop();
	
	
	return 0;
}
