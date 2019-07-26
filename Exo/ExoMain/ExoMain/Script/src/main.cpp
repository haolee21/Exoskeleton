#include<iostream>
#include<cstdio>
#include<thread>
#include<mutex>
#include "Sensor.h"
#include<unistd.h> //for linux sleep
#include "PWM.h"
#include "Controller.h"
#include <chrono>
#include <boost/filesystem.hpp>
#include <boost/date_time.hpp>

#include <string>
#include "Displayer.hpp"

void DelaySys(int waitTime) {
	struct timespec ts2 = { 0 };
	ts2.tv_sec = 0;
	ts2.tv_nsec = 10000000; //10 us
	nanosleep(&ts2, (struct timespec*)NULL);
}
void ReadSenData(Sensor *sensor){
	
	std::cout<<"main"<<sensor->senData[0];
	std::cout<<endl;
}


using namespace std;
int main(void)
{
	//create the folder for result saving
	string filePath;
	{
		boost::posix_time::ptime timeLocal = boost::posix_time::second_clock::local_time();
		
		filePath = "../data/"+to_string(timeLocal.time_of_day().hours())+to_string(timeLocal.time_of_day().minutes())+
		to_string(timeLocal.date().month())+to_string(timeLocal.date().day())+to_string(timeLocal.date().year());
	}
	boost::filesystem::create_directory(filePath);

	wiringPiSetupSys(); //setup the system ports, timer, etc. 

	//define command array
	Com com;
	// const int comLen =2;
	// bool *comArray= new bool[comLen];
	// bool *start = comArray;

	for(int i = 0;i<com.comLen;i++){
		com.comArray[i] = false;

	}
	
	char portName[] = "/dev/ttyACM0";
	Sensor sensor = Sensor(filePath,portName, 1600L,&com);
	std::chrono::system_clock::time_point startTime = std::chrono::system_clock::now();
	sensor.Start(startTime);
	
	
	//connect to pc
	std::cout<<"Do you want to connect to PC? (y/n)\n";
	char ans;
	std::cin>>ans;
	if(ans == 'y' || ans == 'Y'){
		Displayer client = Displayer();
	}
	
	while(true){
		cout<<"Command: ";
		string command;
		cin>>command;
		{
			lock_guard<mutex> lock(com.comLock);
			if(command=="testpwm")
				com.comArray[TESTPWM] = true;
			else if(command=="testval")
				com.comArray[TESTVAL] = true;
			else if(command == "end")
				break;
			else
				cout<<"not such command\n";	
		}
	}
	for(int i=0;i<100;i++){	
		DelaySys(1);
	}
	sensor.Stop();
	




	DelaySys(5);
	
	return 0;
}
