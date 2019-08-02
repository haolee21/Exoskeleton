#include "Controller.h"
#include <iostream>
#include <wiringPi.h>
#include <thread>
#include <queue>

const int NUMSEN = 9;
#define RAWDATALEN 34 //this has to be the same as defined in Sensor.h
typedef std::chrono::duration<long, std::nano> nanosecs_t;
typedef std::chrono::duration<int, std::micro> microsecs_t;
typedef std::chrono::duration<int, std::milli> millisecs_t;
void Controller::ConMainLoop(int *senData, char *senRaw)
{
    queue<thread> taskQue;
    this->senData = senData;
    std::vector<int> curSen(senData + 1, senData + NUMSEN + 1);

    // assign task to controller
    //taskQue.push(std::thread(&Controller::TestReactingTime,this));
    {
        std::lock_guard<std::mutex> lock(this->com->comLock);
        if (this->com->comArray[TESTVAL])
            taskQue.push(std::thread(&Controller::TestValve, this));
        if (this->com->comArray[TESTPWM])
            taskQue.push(std::thread(&Controller::TestPWM, this));
        if (this->com->comArray[SHUTPWM]){
            taskQue.push(std::thread(&Controller::ShutDownPWM,this));
            this->com->comArray[SHUTPWM]=false;
        }
    }

    while (!taskQue.empty())
    {
        taskQue.front().join();
        taskQue.pop();
    }
    if (this->display)
    {
        if (this->preSend)
        {
            char sendData[RAWDATALEN + VALNUM + PWMNUM];
            std::copy(senRaw, senRaw + RAWDATALEN, sendData);
            std::copy(this->valveCond, this->valveCond + VALNUM, sendData + RAWDATALEN);
            std::copy(this->pwmDuty, this->pwmDuty + PWMNUM, sendData + RAWDATALEN + VALNUM);
            this->client->send(sendData, RAWDATALEN + VALNUM + PWMNUM);
            this->preSend = false;
        }
        else
        {
            this->preSend = true;
        }
    }
}

void Controller::TestValve()
{
    if(this->tvParam.curTestCount==this->tvParam.maxTestCount){
        this->tvParam.curTestCount=0;
        if(this->tvParam.singleValCount++<this->tvParam.maxTest){
            if(this->tvParam.curValCond){
                this->ValveOn(this->ValveList[this->tvParam.testValIdx],this->senData[0]);
                this->tvParam.curValCond=false;
            }
            else{
                this->ValveOff(this->ValveList[this->tvParam.testValIdx], this->senData[0]);
                this->tvParam.curValCond = true;
            }

        }
        else{
            this->tvParam.singleValCount=0;
            this->tvParam.testValIdx++;
            if(this->tvParam.testValIdx==VALNUM){
                this->tvParam.testValIdx=0;
            }
        }

    }
    else{
        this->tvParam.curTestCount++;
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

void Controller::TestReactingTime()
{
    if (this->trParam.dataNotSent)
    {
        this->trParam.dataNotSent = false;
        this->trParam.testOut->On(this->senData[0]);
        this->trParam.sendTime = std::chrono::system_clock::now();
    }
    else
    {
        if (this->senData[9] > 300)
        {
            std::chrono::system_clock::time_point curTime = std::chrono::system_clock::now();
            microsecs_t sen_time(std::chrono::duration_cast<microsecs_t>(curTime - this->trParam.sendTime));
            ;
            std::cout << " reaction time = " << sen_time.count() << "us\n";
            this->trParam.testOut->Off(this->senData[0]);
            this->trParam.dataNotSent = true;
        }
    }
}

void Controller::TestPWM()
{
    if(this->tpParam.dutyLoopCount==this->tpParam.preScaler){
        this->tpParam.dutyLoopCount=0;
        if(this->tpParam.curTestDuty==100){
            this->tpParam.curTestDuty=0;
            this->tpParam.testPWMidx++;
            if(this->tpParam.testPWMidx==PWMNUM)
                this->tpParam.testPWMidx=0;
        }
        else
            this->tpParam.curTestDuty+=10;
        this->SetDuty(this->PWMList[this->tpParam.testPWMidx],this->tpParam.curTestDuty,this->senData[0]);
        
    }
    else{
        this->tpParam.dutyLoopCount++;
        
    }
    
    // if(this->tpParam.testPWMidx==PWMNUM){
    //     this->tpParam.testPWMidx = 0;
    // }
    // else
    // {
    //     if (this->tpParam.dutyLoopCount == 100)
    //     {
    //         this->tpParam.dutyLoopCount = 0;
    //         if (this->tpParam.curTestDuty < 100)
    //         {
    //             this->tpParam.curTestDuty += 10;
    //         }
    //         else
    //         {
    //             this->tpParam.curTestDuty = 0;
    //         }
    //     }
    //     else
    //     {
    //         this->tpParam.dutyLoopCount++;
    //     }
    //     this->SetDuty(this->KnePreVal, this->tpParam.curTestDuty, this->senData[0]);
    //     this->SetDuty(this->AnkPreVal, 100 - this->tpParam.curTestDuty, this->senData[0]);
    //     // this->KnePreVal->SetDuty(this->tpParam.curTestDuty,this->senData[0]);
    // }
}
void Controller::ShutDownPWM(){
    this->SetDuty(this->KnePreVal, 0, this->senData[0]);
    this->SetDuty(this->AnkPreVal, 0, this->senData[0]);

}
void Controller::ValveOn(std::shared_ptr<Valve> val, int curTime)
{
    val->On(curTime);
    this->valveCond[val->GetValIdx()] = 'd';
}

void Controller::ValveOff(std::shared_ptr<Valve> val, int curTime)
{
    val->Off(curTime);
    this->valveCond[val->GetValIdx()] = '!';
}
Controller::Controller(std::string filePath, Com *_com, bool _display)
{
    this->testSendCount = 0;
    this->preSend = true;
    this->display = _display;
    if (_display)
    {
        this->client.reset(new Displayer());
        // this->client.reset(new Displayer());
    }

    this->valveCond = new char[VALNUM];
    this->pwmDuty = new char[PWMNUM];

    this->com = _com;

    this->LKneVal1.reset(new Valve("LKneVal1", filePath, OP9, 0));
    this->LKneVal2.reset(new Valve("LKneVal2", filePath, OP4, 1));
    this->LAnkVal1.reset(new Valve("LAnkVal1", filePath, OP6, 2));
    this->LAnkVal2.reset(new Valve("LAnkVal2", filePath, OP7, 3));
    this->BalVal.reset(new Valve("BalVal", filePath, OP10, 4));
    this->LRelVal.reset(new Valve("LRelVal", filePath, OP8, 5));

    // this->KnePreVal = new PWMGen("KnePreVal",filePath,OP1,30000L);
    // this->AnkPreVal = new PWMGen("AnkPreVal",filePath,OP2,30000L);
    this->KnePreVal.reset(new PWMGen("KnePreVal", filePath, OP1, 40000L, 0));
    this->AnkPreVal.reset(new PWMGen("AnkPreVal", filePath, OP2, 40000L, 1));
    this->PWMList[0]=this->KnePreVal;
    this->PWMList[1]=this->AnkPreVal;

    this->ValveList[0] = this->LKneVal1;
    this->ValveList[1] = this->LKneVal2;
    this->ValveList[2] = this->LAnkVal1;
    this->ValveList[3] = this->LAnkVal2;
    this->ValveList[4] = this->BalVal;
    this->ValveList[5] = this->LRelVal;

    std::shared_ptr<PWMGen> *begPWM=this->PWMList;
    do{
        this->SetDuty(*begPWM,0,0);
    }while(++begPWM!=std::end(this->PWMList));
    this->SetDuty(this->KnePreVal, 0, 0);
    this->SetDuty(this->AnkPreVal, 0, 0);

    this->knePreValTh = this->KnePreVal->Start();
    this->ankPreValTh = this->AnkPreVal->Start();

    std::shared_ptr<Valve> *begVal = this->ValveList;
    do
    {
        this->ValveOff(*begVal, 0);
    } while (++begVal != std::end(this->ValveList));

    this->trParam.testOut.reset(new Valve("TestMea", filePath, 8, 6)); //this uses gpio2
    this->ValveOff(this->trParam.testOut, 0);
}
void Controller::SetDuty(std::shared_ptr<PWMGen> pwmVal, int duty, int curTime)
{
    pwmVal->SetDuty(duty, curTime);
    this->pwmDuty[pwmVal->GetIdx()] = pwmVal->duty.byte[0];
}

Controller::~Controller()
{
    this->KnePreVal->Stop();
    this->AnkPreVal->Stop();

    this->knePreValTh->join();
    this->ankPreValTh->join();
    delete valveCond;
    delete pwmDuty;
}
