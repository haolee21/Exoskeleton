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
void Controller::ConMainLoop(unsigned int *_senData, char *senRaw)
{
    queue<thread> taskQue;
    // this->senData = senData;
    this->senData=_senData;
    std::vector<unsigned int> curSen(_senData + 1, _senData + NUMSEN + 1);

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
        if (this->com->comArray[ENGRECL]){
            taskQue.push(std::thread(&Controller::FSM_loop,this));
        }
        if(this->com->comArray[KNEMODSAMP]){
            taskQue.push(std::thread(&Controller::SampKneMod,this,com->comVal[KNEMODSAMP]));
        }
        if(this->com->comArray[KNEPREREL]){
            taskQue.push(std::thread(&Controller::KneRel,this));
        }
    }

    while (!taskQue.empty())
    {
        taskQue.front().join();
        taskQue.pop();
    }
    if (this->display)
    {
        // if (this->preSend)
        // {
        if(this->preSend==this->dispPreScale){
            char sendData[RAWDATALEN + VALNUM + PWMNUM];
            std::copy(senRaw, senRaw + RAWDATALEN, sendData);
            std::copy(this->valveCond, this->valveCond + VALNUM, sendData + RAWDATALEN);
            std::copy(this->pwmDuty, this->pwmDuty + PWMNUM, sendData + RAWDATALEN + VALNUM);
            this->client->send(sendData, RAWDATALEN + VALNUM + PWMNUM);
            this->preSend = 0;

        }
        else
        {
            this->preSend++;
        //     this->preSend = true;
        }
    }
}

void Controller::TestValve()
{
    if(this->tvParam.curTestCount==this->tvParam.maxTestCount){
        this->tvParam.curTestCount=0;
        if(this->tvParam.singleValCount++<this->tvParam.maxTest){
            if(this->tvParam.curValCond){
                this->ValveOn(this->ValveList[this->tvParam.testValIdx]);
                this->tvParam.curValCond=false;
            }
            else{
                this->ValveOff(this->ValveList[this->tvParam.testValIdx]);
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
        this->SetDuty(this->PWMList[this->tpParam.testPWMidx],this->tpParam.curTestDuty);
        
    }
    else{
        this->tpParam.dutyLoopCount++;
        
    }
}
void Controller::ShutDownPWM(){

    std::shared_ptr<PWMGen> *begPWM=this->PWMList;
    do{
        this->SetDuty(*begPWM,0);
        (*begPWM)->Start();
    }while(++begPWM!=std::end(this->PWMList));
    

}
void Controller::ValveOn(std::shared_ptr<Valve> val)
{
    val->On(this->senData[TIME]);
    this->valveCond[val->GetValIdx()] = 'd';
}

void Controller::ValveOff(std::shared_ptr<Valve> val)
{
    val->Off(this->senData[TIME]);
    this->valveCond[val->GetValIdx()] = '!';
}
Controller::Controller(std::string filePath, Com *_com, bool _display,std::chrono::system_clock::time_point _origin)
{

    this->senData=new unsigned int(0);

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

    this->LKneVal.reset(new Valve("LKneVal", filePath, OP5, 0));
    this->RKneVal.reset(new Valve("RKneVal", filePath, OP8, 1));
    this->LAnkVal.reset(new Valve("LAnkVal", filePath, OP6, 2));
    this->RAnkVal.reset(new Valve("RAnkVal", filePath, OP9, 3));
    this->LBalVal.reset(new Valve("LBalVal", filePath, OP7, 4));
    this->RBalVal.reset(new Valve("RBalVal", filePath, OP10, 5));


    this->RelVal.reset(new Valve("RelVal", filePath, OP16, 6));

    // this->KnePreVal = new PWMGen("KnePreVal",filePath,OP1,30000L);
    // this->AnkPreVal = new PWMGen("AnkPreVal",filePath,OP2,30000L);
    this->LKnePreVal.reset(new PWMGen("LKnePreVal", filePath, OP1, 40000L, 0,_origin));
    this->LAnkPreVal.reset(new PWMGen("LAnkPreVal", filePath, OP2, 40000L, 1,_origin));
    this->RKnePreVal.reset(new PWMGen("RKnePreVal", filePath, OP3, 40000L, 2,_origin));
    this->RAnkPreVal.reset(new PWMGen("RAnkPreVal", filePath, OP4, 40000L, 3,_origin));

    this->PWMList[0]=this->LKnePreVal;
    this->PWMList[1]=this->LAnkPreVal;
    this->PWMList[2]=this->RKnePreVal;
    this->PWMList[3]=this->RAnkPreVal;

    this->ValveList[0] = this->LKneVal;
    this->ValveList[1] = this->RKneVal;
    this->ValveList[2] = this->LAnkVal;
    this->ValveList[3] = this->RAnkVal;
    this->ValveList[4] = this->LBalVal;
    this->ValveList[5] = this->RBalVal;
    this->ValveList[6] = this->RelVal;

    std::shared_ptr<PWMGen> *begPWM=this->PWMList;
    do{
        this->SetDuty(*begPWM,0);
        (*begPWM)->Start();
    }while(++begPWM!=std::end(this->PWMList));


    std::shared_ptr<Valve> *begVal = this->ValveList;
    do
    {
        this->ValveOff(*begVal);
    } while (++begVal != std::end(this->ValveList));

    // this->trParam.testOut.reset(new Valve("TestMea", filePath, 8, 6)); //this uses gpio2
    // this->ValveOff(this->trParam.testOut);
}
void Controller::SetDuty(std::shared_ptr<PWMGen> pwmVal, int duty)
{
    pwmVal->SetDuty(duty, this->senData[TIME]);
    this->pwmDuty[pwmVal->GetIdx()] = pwmVal->duty.byte[0];
}

Controller::~Controller()
{
    this->PreRel();
    std::shared_ptr<PWMGen> *begPWM=this->PWMList;
    do{
        
        (*begPWM)->Stop();
    }while(++begPWM!=std::end(this->PWMList));
    
    delete this->valveCond;
    delete this->pwmDuty;
    //delete this->senData;
}

void Controller::FSM_loop()
{
    
    switch (this->LEngRec.curPhase)
    {
    case 1:
        this->LEngRec.curPhase = this->Phase1Con();
        break;
    case 2:
        this->LEngRec.curPhase = this->Phase2Con();
        break;
    case 3:
        this->LEngRec.curPhase = this->Phase3Con();
        break;
    case 4:
        this->LEngRec.curPhase = this->Phase4Con();
        break;
    case 5:
        this->LEngRec.curPhase = this->Phase5Con();
        break;
    case 6:
        this->LEngRec.curPhase = this->Phase6Con();
        break;
    case 7:
        this->LEngRec.curPhase = this->Phase7Con();
        break;
    case 8:
        this->LEngRec.curPhase = this->Phase8Con();
        break;
    default:
        break;
    }
}
int Controller::Phase1Con()
{
    // if(this->senData[LKNEPRE]<this->sup_LKnePre){
    //     this->ValveOn(this->BalVal);
    //     this->ValveOff(this->LKneVal1);
    //     this->ValveOn(this->LKneVal2);
    //     this->SetDuty(this->KnePreVal,this->CalDuty(this->senData[LKNEPRE],this->sup_LKnePre,this->senData[TANKPRE]));
        
    //     return 1;
    // }
    // else{
    //     this->KnePreVal->SetDuty(0,this->senData[TIME]);
    //     return 2;
    // }
    return 1;
}
int Controller::Phase2Con()
{
    
    return 2;
}
int Controller::Phase3Con()
{
    
    return 3;
}
int Controller::Phase4Con()
{
    
    return 4;
}
int Controller::Phase5Con()
{
    
    return 5;
}
int Controller::Phase6Con()
{
    
    return 6;
}
int Controller::Phase7Con()
{
    
    return 7;
}
int Controller::Phase8Con()
{
    return 8;
}
int Controller::CalDuty(unsigned int curPre, unsigned int desPre,unsigned int tankPre){
    if(curPre>desPre)
        return 0;
    else{
        if(tankPre>curPre){
            if(tankPre>desPre){
                int duty = (desPre-curPre)*100/(tankPre-curPre);
                std::cout<<"duty need: "<<duty<<std::endl;
                return duty;
            }
            else
                return 0;
        }
        else
            return 0;
        
    }
    
}

void Controller::PreRel(){
    std::cout<<"Pressure Release\n";
    this->ValveOff(this->LKneVal);
    this->ValveOff(this->RKneVal);
    this->SetDuty(this->LKnePreVal,100);
    this->SetDuty(this->RKnePreVal,100);

    this->ValveOff(this->LAnkVal);
    this->ValveOff(this->RAnkVal);

    this->SetDuty(this->LAnkPreVal,100);
    this->SetDuty(this->RAnkPreVal,100);

    sleep(RELTIME);

    this->SetDuty(this->LKnePreVal,0);
    this->SetDuty(this->LAnkPreVal,0);
    this->SetDuty(this->RKnePreVal,0);
    this->SetDuty(this->RAnkPreVal,0);
    this->LKnePreVal->SetDuty(0,senData[TIME]+RELTIME*1000000);
    this->LAnkPreVal->SetDuty(0,senData[TIME]+RELTIME*1000000);
    this->RKnePreVal->SetDuty(0,senData[TIME]+RELTIME*1000000);
    this->RAnkPreVal->SetDuty(0,senData[TIME]+RELTIME*1000000);
}
void Controller::SampKneMod(int testDuty){
    // if(this->sampKneMod.outLoopCount==0){
    //     if(this->sampKneMod.cycleCount==0){
    //         this->ValveOn(this->BalVal);
    //         this->ValveOff(this->LKneVal1);
    //         this->ValveOn(this->LKneVal2);

    //         this->SetDuty(this->KnePreVal,testDuty);
    //         this->sampKneMod.cycleCount++;
    //     }
    //     else{
    //         if(this->sampKneMod.cycleCount==this->sampKneMod.maxCycle){
    //             this->sampKneMod.cycleCount=0;
                
    //             // this->ValveOff(this->BalVal);
    //             // this->ValveOff(this->LKneVal1);
    //             // this->ValveOff(this->LKneVal2);

    //             this->SetDuty(this->KnePreVal,0);
    //             this->sampKneMod.outLoopCount++;
    //         }
    //         else
    //             this->sampKneMod.cycleCount++;
    //     }
    // }
    // else
    // {
    //     if(this->sampKneMod.outLoopCount==this->sampKneMod.maxOuterLoop){
    //         this->sampKneMod.outLoopCount=0;
    //         // this->com->comArray[KNEMODSAMP]=false;
    //     }
    //     else{
            
    //         this->sampKneMod.outLoopCount++;
    //     }
    // }
    

}
void Controller::KneRel(){
    // if(this->knePreRel.curRelCycle!=this->knePreRel.maxRelCycle){
    //     if(this->knePreRel.curRelCycle==0){
    //         this->ValveOff(this->LKneVal1);
    //         this->ValveOff(this->LKneVal2);
    //         this->ValveOff(this->LRelVal);
    //         this->ValveOff(this->BalVal);
    //     }
    //     this->knePreRel.curRelCycle++;
    // }
    // else{
    //     this->ValveOn(this->BalVal);
    //     this->ValveOff(this->LKneVal1);
    //     this->ValveOn(this->LKneVal2);
    //     this->com->comArray[KNEPREREL]=false;
    //     this->knePreRel.curRelCycle=0;

    // }
}