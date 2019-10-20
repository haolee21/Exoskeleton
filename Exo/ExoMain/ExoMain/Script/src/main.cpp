
#include "common.hpp"
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
#include <iomanip>



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
	// Eigen::MatrixXd m(2,2);
  	// m(0,0) = 3;
  	// m(1,0) = 2.5;
  	// m(0,1) = -1;
  	// m(1,1) = m(1,0) + m(0,1);
  	// std::cout << m << std::endl;

	

	std::cout<<"Do you want to connect to PC? (y/n)\n";
	char ans;
	bool display = false;
	std::cin>>ans;

	if(ans == 'y' || ans == 'Y'){
		//std::cout<<"create displayer\n";
		display = true;
	}

	// We create directory here since the raspberry pi will sync its time with pc during connection
	//create the folder for result saving
	//string homeFolder = "../data";
	//string homeFolder = "/home/pi/Data/ExpData";
	string homeFolder = "/media/pi/ExoData";
	if(!boost::filesystem::exists(homeFolder)){
		//boost::filesystem::create_directory(homeFolder);
		homeFolder = "/home/pi/Data";
		if(!boost::filesystem::exists(homeFolder)){
			boost::filesystem::create_directory(homeFolder);
		}
		//if there is no usb, directly save on sd card
	}
	string filePath;
	{
		boost::posix_time::ptime timeLocal = boost::posix_time::second_clock::local_time();
		stringstream hour;
		hour<<setw(2)<<std::setfill('0')<<to_string(timeLocal.time_of_day().hours());
		stringstream min;
		min<<setw(2)<<std::setfill('0')<<to_string(timeLocal.time_of_day().minutes());
		stringstream date;
		date<<setw(2)<<setfill('0')<<to_string(timeLocal.date().day());
		stringstream month;
		month<<setw(2)<<setfill('0')<<to_string(timeLocal.date().month());
		// filePath = "../data/"+to_string(timeLocal.time_of_day().hours())+to_string(timeLocal.time_of_day().minutes())+
		// to_string(timeLocal.date().month())+to_string(timeLocal.date().day())+to_string(timeLocal.date().year());
		filePath = homeFolder+'/'+to_string(timeLocal.date().year())+'-'+month.str()+date.str()+'-'+hour.str()+min.str();
	}
	boost::filesystem::create_directory(filePath);


	//define command array
	Com com;


	for(int i = 0;i<com.comLen;i++){
		com.comArray[i] = false;

	}
	
	char portName[] = "/dev/ttyACM0";
	Sensor sensor = Sensor(filePath,portName, SAMPTIME,&com,display);
	std::chrono::system_clock::time_point startTime = std::chrono::system_clock::now();
	sensor.Start(startTime);
	
	
	
	
	while(true){
		cout<<"Command: ";
		string command;
		cin>>command;
		{
			lock_guard<mutex> lock(com.comLock);
			if(command=="testpwm"){
				if (com.comArray[TESTPWM]==false)
					com.comArray[TESTPWM]=true;
				else{
					com.comArray[TESTPWM]=false;
					com.comArray[SHUTPWM]=true;

				}
					 
			}
			else if(command=="testval")
				com.comArray[TESTVAL] = !com.comArray[TESTVAL];
			
			
			else if(command.substr(0,4)=="samp"){
				com.comArray[KNEMODSAMP] = !com.comArray[KNEMODSAMP];
				stringstream numVal(command.substr(4,5));
				int num;
				numVal>>num;
				com.comVal[KNEMODSAMP]=num;
			}
			else if(command=="knerel"){
				com.comArray[KNEPREREL] = true;
			}
			else if(command == "testallleak"){
				com.comArray[TESTALLLEAK] = true;
			}
			else if(command=="freewalk"){
				com.comArray[FREEWALK]=true;
			}

			
			else if(command=="testleak1"){
				com.comArray[TESTLEAK] = true;
				com.comVal[TESTLEAK]=1;
			}
			else if(command=="testleak2"){
				com.comArray[TESTLEAK] = true;
				com.comVal[TESTLEAK]=2;
			}
			else if(command=="testleak3"){
				com.comArray[TESTLEAK]=true;
				com.comVal[TESTLEAK]=3;
			}
			else if(command=="testleak4"){
				com.comArray[TESTLEAK]=true;
				com.comVal[TESTLEAK]=4;
			}
			else if(command=="testleak5"){
				com.comArray[TESTLEAK]=true;
				com.comVal[TESTLEAK]=5;
			}
			else if(command=="testleak6"){
				com.comArray[TESTLEAK]=true;
				com.comVal[TESTLEAK]=6;
			}
			else if(command == "end")
				break;
			else if(command =="testlank"){
				com.comArray[TESTLANK]=true;
			}
			else if(command == "testrank"){
				com.comArray[TESTRANK]=true;
			}
			else if(command =="r"){
				com.comArray[SHOWSEN]=true;
			}
			else if(command == "bipedrec"){
				com.comArray[BIPEDREC] = !com.comArray[BIPEDREC];
				if(com.comArray[BIPEDREC]==true)
					std::cout<<"Biped energy recycle on\n";
				else
				{
					std::cout<<"Biped energy recycle off\n";
				}
				
			}
			else if(command == "testsync"){
				com.comArray[TESTSYNC] = true;
			}
			else if(command == "pidtestlk"){
				com.comArray[PIDACTTEST] = true;
				com.comVal[PIDACTTEST] =0;
			}
			else if(command =="pidtestla"){
				com.comArray[PIDACTTEST]=true;
				com.comVal[PIDACTTEST]=1;
			}
			else if(command =="pidtestrk" ){
				com.comArray[PIDACTTEST]=true;
				com.comVal[PIDACTTEST]=2;
			}
			else if(command =="pidtestra"){
				com.comArray[PIDACTTEST]=true;
				com.comVal[PIDACTTEST]=3;
			}
			else
				cout<<"not such command\n";	
		}
	}
	
	sensor.Stop();
	




	DelaySys(5);
	cout<<"File save to: "<<filePath<<endl;
	return 0;
}
