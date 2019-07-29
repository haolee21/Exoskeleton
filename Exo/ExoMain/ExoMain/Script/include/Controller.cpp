#include "Controller.h"
#include <iostream>
#include <wiringPi.h>
#include <thread>
#include <queue>
const int NUMSEN = 9;
typedef std::chrono::duration<long, std::nano> nanosecs_t;
typedef std::chrono::duration<int, std::micro> microsecs_t;
typedef std::chrono::duration<int, std::milli> millisecs_t;
void Controller::ConMainLoop(int *senData){
    queue<thread> taskQue;
    this->senData = senData;
    std::vector<int> curSen(senData+1,senData+NUMSEN+1);
    this->conRec->PushData((unsigned long)senData[0],curSen);

    // assign task to controller
    //taskQue.push(std::thread(&Controller::TestReactingTime,this));
    {
        std::lock_guard<std::mutex> lock(this->com->comLock);
        if(this->com->comArray[TESTVAL])
            taskQue.push(std::thread(&Controller::TestValve,this));
        if(this->com->comArray[TESTPWM])
            taskQue.push(std::thread(&Controller::TestPWM,this));
    }
    
    while(!taskQue.empty()){
        taskQue.front().join();
        taskQue.pop();
    }
    
}

void Controller::TestValve()
{
    //we will on/off each valve 20 times
    if(this->tvParam.singleValCount++<this->tvParam.maxTest){
        if(this->tvParam.curValCond){
            this->ValveList[this->tvParam.testValIdx]->On(this->senData[0]);
            this->tvParam.curValCond = false;
        }
        else{
            this->ValveList[this->tvParam.testValIdx]->Off(this->senData[0]);
            this->tvParam.curValCond = true;
        } 
    }
    else{
        this->tvParam.singleValCount = 0;
        this->tvParam.testValIdx++;
        this->tvParam.curValCond = true;
        if(this->tvParam.testValIdx==this->ValNum){
            this->tvParam.testValIdx = 0;
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



void Controller::TestReactingTime(){
    if(this->trParam.dataNotSent){
        this->trParam.dataNotSent = false;
        this->trParam.testOut->On(this->senData[0]);
        this->trParam.sendTime = std::chrono::system_clock::now();
    }
    else{
        if(this->senData[9]>300){
            std::chrono::system_clock::time_point curTime = std::chrono::system_clock::now(); 
            microsecs_t sen_time(std::chrono::duration_cast<microsecs_t>(curTime - this->trParam.sendTime));;
            std::cout<<" reaction time = "<< sen_time.count()<< "us\n";
            this->trParam.testOut->Off(this->senData[0]);
            this->trParam.dataNotSent = true;
        }

    }
    
}
void Controller::TestPWM(){
    if(this->tpParam.notStart){
        this->KnePreVal->SetDuty(0,this->senData[0]);
        this->AnkPreVal->SetDuty(0,this->senData[0]);
        this->tpParam.notStart = false;
        //this->KnePreVal->Start();
    }
    else
    {
        if(this->tpParam.dutyLoopCount==100){
            this->tpParam.dutyLoopCount=0;
            if(this->tpParam.curTestDuty<100){
                this->tpParam.curTestDuty+=10;
            }
            else{
                this->tpParam.curTestDuty = 0;
            }
        }
        else
        {
            this->tpParam.dutyLoopCount++;
        }
        this->KnePreVal->SetDuty(this->tpParam.curTestDuty,this->senData[0]);
        
    }
}
Controller::Controller(std::string filePath,Com *_com)
{
    this->com = _com;

    this->LKneVal1=new Valve("LKneVal1",filePath,OP9);
    this->LKneVal2=new Valve("LKneVal2",filePath,OP4);
    this->LAnkVal1=new Valve("LAnkVal1",filePath,OP6);
    this->LAnkVal2=new Valve("LAnkVal2",filePath,OP7);
    this->BalVal=new Valve("BalVal",filePath,OP10);
    this->LRelVal=new Valve("LRelVal",filePath,OP8);
    this->KnePreVal = new PWMGen("KnePreVal",filePath,OP1,30000L);
    this->AnkPreVal = new PWMGen("AnkPreVal",filePath,OP2,30000L);

    this->ValveList[0] = this->LKneVal1;
    this->ValveList[1] = this->LKneVal2;
    this->ValveList[2] = this->LAnkVal1;
    this->ValveList[3] = this->LAnkVal2;
    this->ValveList[4] = this->BalVal;
    this->ValveList[5] = this->LRelVal;
    
    this->KnePreVal->SetDuty(0,0);
    this->AnkPreVal->SetDuty(0,0);
    this->knePreValTh = this->KnePreVal->Start();
    this->ankPreValTh = this->AnkPreVal->Start();
   
   
    
    this->conRec = new Recorder<int>("con",filePath,"time,sen1,sen2,sen3,sen4,sen5,sen6,sen7,sen8,sen9");
    
    //turn off all the valve
    Valve **begVal = this->ValveList;
    do{
        (*begVal)->Off(0);
    }while(++begVal!=std::end(this->ValveList));
 
    
    this->trParam.testOut = new Valve("TestMea",filePath,8); //this uses gpio2
    this->trParam.testOut->Off(0);
    
}

Controller::~Controller()
{
    this->KnePreVal->Stop();
    this->AnkPreVal->Stop();

    this->knePreValTh->join();
    this->ankPreValTh->join();
    delete this->KnePreVal;
    delete this->AnkPreVal;
    delete this->conRec;
    Valve **begVal = this->ValveList;
    do{
        
        delete (*begVal);
    }while(++begVal!=std::end(this->ValveList));
}
