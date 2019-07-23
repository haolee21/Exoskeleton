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
mutex SenLock;
void DelaySys(int waitTime) {
	struct timespec ts2 = { 0 };
	ts2.tv_sec = 0;
	ts2.tv_nsec = 10000000; //10 us
	nanosleep(&ts2, (struct timespec*)NULL);
}
void ReadSenData(Sensor *sensor){
	std::lock_guard<std::mutex> lock(SenLock);
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

	
	char portName[] = "/dev/ttyACM0";
	Sensor sensor = Sensor(filePath,portName, 1600L,&SenLock);
	std::chrono::system_clock::time_point startTime = std::chrono::system_clock::now();
	sensor.Start(startTime);
	cout << "finish creating" << endl;
	//Controller con = Controller(&sensor,&SenLock);
	//con.TestValve();
	
	for(int i=0;i<1000;i++){
		//ReadSenData(&sensor);
		DelaySys(1);
	}
	
	
	sensor.Stop();
	
	//print the control result
	// for(int i=1;i<con.ValveList[0].recIndex;i++){
	// 	std::cout<<con.ValveList[0].valTimeRec[i]<<","<<con.ValveList[0].valCondRec[i]<<std::endl;
	// }
	//print sense result
	// int time0 = 0;
	// for(int i=0;i<sensor.recIndex;i++){
	// 	for (int k=0;k<NUMSEN;k++){
	// 		if (k==0){
	// 			std::cout<<sensor.totSenRec[i][k]-time0;
	// 			time0 = sensor.totSenRec[i][k];
	// 		}
	// 		else
	// 			std::cout<<sensor.totSenRec[i][k]<<",";
	// 	}
	// 	std::cout<<std::endl;

	// }

	//only print time
	// int time0 = sensor.totSenRec[0][0];
	// for (int i=0;i<sensor.recIndex-1;i++){
	// 	std::cout<<sensor.totSenRec[i][0]-time0<<std::endl;
	// 	time0 = sensor.totSenRec[i][0];
	// }
	// std::cout<<"finish"<<std::endl;

	DelaySys(5);
	return 0;
}
