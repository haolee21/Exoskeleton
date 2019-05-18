#include "Controller.h"
#include <iostream>
#include<wiringPi.h>
void Controller::TestValve(){
    
    for(int i=0;i<this->ValNum;i++){
        
        std::cout<<"Test Valve "<<i<<std::endl;
        for(int i2=0;i2<20;i2++){
            this->ValveList[i].On();
            this->Sleep(200);
            this->ValveList[i].Off();
            this->Sleep(200);
        }

    }  
}
void Controller::Sleep(int sleepTime){
    std::chrono::system_clock::time_point startTime = std::chrono::system_clock::now(); //starting time
    while (std::chrono::system_clock::now()-startTime
			< std::chrono::milliseconds(sleepTime)) {
			this->WaitToSync(); 
		}


}
void Controller::WaitToSync(){
    struct timespec ts = {0};
    ts.tv_sec=0;
    ts.tv_nsec = 10000L;
    nanosleep(&ts,(struct timespec*)NULL);
}
Controller::Controller(Sensor *sensor,mutex *senLock)
{
    this->sensor = sensor;
    this->senLock = senLock;
    std::chrono::system_clock::time_point startTime = std::chrono::system_clock::now();
    
    //turn off all the valves and set start time
    for(int i=0;i<this->ValNum;i++){
        this->ValveList[i].SetStartTime(startTime);
        this->ValveList[i].Off();
    }


    
}


Controller::~Controller()
{
}