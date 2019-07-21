#include "Controller.h"
#include <iostream>
#include <wiringPi.h>

const int NUMSEN = 9;
typedef std::chrono::duration<long, std::nano> nanosecs_t;
typedef std::chrono::duration<int, std::micro> microsecs_t;
typedef std::chrono::duration<int, std::milli> millisecs_t;
void Controller::ConMainLoop(int *senData){
   

    std::vector<int> curSen(senData+1,senData+NUMSEN+1);
    this->senRec->PushData((unsigned long)senData[0],curSen);
    
    // std::cout<<"current sensor value: "
    //     <<curSen[0]<<','<<curSen[1]<<','<<curSen[2]<<','<<curSen[3]<<','<<curSen[4]<<','
    //     <<curSen[5]<<','<<curSen[6]<<','<<curSen[7]<<','<<curSen[8]<<std::endl;

}



void Controller::TestValve()
{

    for (int i = 0; i < this->ValNum; i++)
    {

        std::cout << "Test Valve " << i << std::endl;
        for (int i2 = 0; i2 < 20; i2++)
        {
            this->ValveList[i].On();
            this->Sleep(200);
            this->ValveList[i].Off();
            this->Sleep(200);
        }
    }
}
void Controller::Sleep(int sleepTime)
{
    std::chrono::system_clock::time_point startTime = std::chrono::system_clock::now(); //starting time
    while (std::chrono::system_clock::now() - startTime < std::chrono::milliseconds(sleepTime))
    {
        this->WaitToSync();
    }
}
void Controller::WaitToSync()
{
    struct timespec ts = {0};
    ts.tv_sec = 0;
    ts.tv_nsec = 10000L;
    nanosleep(&ts, (struct timespec *)NULL);
}

void Controller::Wait(long waitMilli)
{
    struct timespec ts = {0};
    ts.tv_sec = 0;
    ts.tv_nsec = waitMilli*1000000;
    nanosleep(&ts, (struct timespec *)NULL);
}
bool Controller::SendTestMeasurement(bool testState)
{
    //This will control the output pin with desired testState
    if(testState){
        this->testOut.On();
    }
    else{
        this->testOut.Off();
    }
}
bool Controller::WaitTestMeasurement(std::chrono::system_clock::time_point &senseTime,bool &testState,int *senData){
    if(testState){
        if(senData[9]>300){
            senseTime = std::chrono::system_clock::now(); 
            testState = false;
            return true;
        }
        else
        {
            return false;
        }
    }
    else{
        if(senData[9]<200){
            senseTime = std::chrono::system_clock::now(); 
            testState = true;
            return true;
        }
        else{
            return false;
        }
    }
    return false;

}
Controller::Controller()
{
    std::cout<<"start to create controller\n";
    std::chrono::system_clock::time_point startTime = std::chrono::system_clock::now(); 
    //turn off all the valves and set start time
    // for (int i = 0; i < this->ValNum; i++)
    // {
    //     this->ValveList[i].SetStartTime(startTime);
    //     this->ValveList[i].Off();
    // }
    std::cout<<"valve list created\n";
    this->senRec = new Recorder<int>("con","time,sen1,sen2,sen3,sen4,sen5,sen6,sen7,sen8,sen9");
}

Controller::~Controller()
{
    std::cout<<"controller destory recorder\n";
    delete this->senRec;
}
