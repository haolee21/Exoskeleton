#include "Controller.h"
#include <iostream>
#include <wiringPi.h>
#include <thread>
#include <queue>


#define RAWDATALEN 34 //this has to be the same as defined in Sensor.h
typedef std::chrono::duration<long, std::nano> nanosecs_t;
typedef std::chrono::duration<int, std::micro> microsecs_t;
typedef std::chrono::duration<int, std::milli> millisecs_t;
void Controller::ConMainLoop(int *_senData, char *senRaw)
{
    
    queue<thread> taskQue;
    // this->senData = senData;
    
    std::memcpy(this->senData,_senData,std::size_t((NUMSEN+1)*sizeof(int)));
    
    
    
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
        
        if(this->com->comArray[KNEMODSAMP]){
            taskQue.push(std::thread(&Controller::SampKneMod,this,com->comVal[KNEMODSAMP]));
        }
        if(this->com->comArray[KNEPREREL]){
            taskQue.push(std::thread(&Controller::KneRel,this));
        }
        if(this->com->comArray[TESTALLLEAK]){
            taskQue.push(std::thread(&Controller::TestAllLeak,this));
            this->com->comArray[TESTALLLEAK] = false;
        }
        if(this->com->comArray[FREEWALK]){
            taskQue.push(std::thread(&Controller::FreeWalk,this));
            this->com->comArray[FREEWALK] =false;
        }
        if(this->com->comArray[TESTLEAK]){
            taskQue.push(std::thread(&Controller::TestLeak,this,com->comVal[TESTLEAK]));
            this->com->comArray[TESTLEAK] = false;
        }
        if(this->com->comArray[TESTLANK]){
            taskQue.push(std::thread(&Controller::TestLAnk,this));
            this->com->comArray[TESTLANK]=false;
        }
        if(this->com->comArray[TESTRANK]){
            taskQue.push(std::thread(&Controller::TestRAnk,this));
            this->com->comArray[TESTRANK]=false;
        }
        if(this->com->comArray[SHOWSEN]){
            taskQue.push(std::thread(&Controller::ShowSen,this));
            this->com->comArray[SHOWSEN]=false;
        }
        if(this->com->comArray[BIPEDREC]){
            taskQue.push(std::thread(&Controller::BipedEngRec,this));
        }
        if(this->com->comArray[TESTSYNC]){
            taskQue.push(std::thread(&Controller::TestReactingTime,this));

        }
    }

    while (!taskQue.empty())
    {
        taskQue.front().join();
        taskQue.pop();
    }
    if (this->display)
    {
        
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
    std::memcpy(this->preSen,this->senData,std::size_t((NUMSEN+1)*sizeof(int)));
    
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
        this->trParam.testOut->On(0); //we do not use this->ValveOn since we don't need this in valve condition
        this->trParam.dataNotSent = false;
        this->trParam.sendTime = std::chrono::system_clock::now();
    }
    else
    {
        if (this->senData[SYNCREAD] > 300)
        {
            std::chrono::system_clock::time_point curTime = std::chrono::system_clock::now();
            microsecs_t sen_time(std::chrono::duration_cast<microsecs_t>(curTime - this->trParam.sendTime));
            
            std::cout << " reaction time = " << sen_time.count() << "us\n";
            this->trParam.testOut->Off(1);
            this->trParam.dataNotSent = true;
            this->com->comArray[TESTSYNC] = false;
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

    this->senData[0]=0;
    this->preSen[0] =0; 
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

    this->trParam.testOut.reset(new Valve("SyncOut",filePath,SYNCOUT,7)); //this is not a valve, the valve index is meaningless, we do not count it in VALNUM
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
    this->curState = 0;
    this->FSM = FSMachine();
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



int Controller::CalDuty(int curPre, int desPre,int tankPre){
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
    this->ValveOn(this->RelVal);
    this->ValveOff(this->LAnkVal);
    this->ValveOff(this->RAnkVal);

    this->SetDuty(this->LAnkPreVal,100);
    this->SetDuty(this->RAnkPreVal,100);

    sleep(RELTIME);

    this->SetDuty(this->LKnePreVal,0);
    this->SetDuty(this->LAnkPreVal,0);
    this->SetDuty(this->RKnePreVal,0);
    this->SetDuty(this->RAnkPreVal,0);
    this->ValveOff(this->RelVal);
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
void Controller::TestLeak(int curPath){
    switch (curPath)
    {
        case 0: //test LKneeSup
            std::cout<<"test LKneeSup\n";
            this->ValveOn(this->LKneVal);
            this->ValveOn(this->LAnkVal);
            this->SetDuty(this->LKnePreVal,100);
            sleep(10);
            this->ValveOff(this->LKneVal);
            this->ValveOff(this->LAnkVal);
            this->SetDuty(this->LKnePreVal,0);
            break;
        case 2: //test LKneeFree and LKneeSup
            std::cout<<"test LKneeFree and LKneeSup\n";
            this->ValveOff(this->LKneVal);
            this->ValveOff(this->LAnkVal);
            this->SetDuty(this->LKnePreVal,100);
            sleep(10);
            this->SetDuty(this->LKnePreVal,0);
            break;
        case 3: //test RKneeSup
            std::cout<<"test RKneeSup\n";
            this->ValveOn(this->RKneVal);
            this->ValveOn(this->RAnkVal);
            this->SetDuty(this->RKnePreVal,100);
            sleep(10);
            this->ValveOff(this->RKneVal);
            this->ValveOff(this->RAnkVal);
            this->SetDuty(this->RKnePreVal,0);
            break;
        case 4://test RKneeFree and RKneeSup
            std::cout<<"test RKneeFree and RKneeSup\n";
            this->ValveOff(this->RKneVal);
            this->ValveOff(this->RAnkVal);
            this->SetDuty(this->RKnePreVal,100);
            sleep(10);
            this->SetDuty(this->RKnePreVal,0);
            break;
        case 5: //test LAnkleSup
            std::cout<<"test LAnkleSup\n";
            this->SetDuty(this->LAnkPreVal,100);
            sleep(10);
            this->SetDuty(this->LAnkPreVal,0);
            break;
        case 6: //test LAnkleFree and LKneeSup (looks weird but that is how the loop is connected)
            break;
    }
    

}
void Controller::TestAllLeakOn(){
    std::cout<<"Leak test started\n";
    this->ValveOff(this->LKneVal);
    this->ValveOff(this->RKneVal);
    this->SetDuty(this->LKnePreVal,100);
    this->SetDuty(this->RKnePreVal,100);
    this->ValveOff(this->LBalVal);
    this->ValveOff(this->RBalVal);
    this->ValveOff(this->LAnkVal);
    this->ValveOff(this->RAnkVal);
    this->SetDuty(this->LAnkPreVal,100);
    this->SetDuty(this->RAnkPreVal,100);
}
void Controller::TestAllLeakOff(){
    this->SetDuty(this->LKnePreVal,0);
    this->SetDuty(this->RKnePreVal,0);
    this->SetDuty(this->LAnkPreVal,0);
    this->SetDuty(this->RAnkPreVal,0);
    this->com->comArray[TESTALLLEAK] = false;
    std::cout<<"leak test ends\n";
}
void Controller::TestAllLeak(){
    if(this->testLeak.all_on){
        this->TestAllLeakOff();
        this->testLeak.all_on = false;
    }
    else
    {
        this->TestAllLeakOn();
        this->testLeak.all_on = true;
    }
    
}
void Controller::FreeWalk_on(){
    //call TestAllLeak with RelVal open

    this->TestAllLeakOn();
    this->ValveOn(this->RelVal);
    
}
void Controller::FreeWalk_off(){
    this->TestAllLeakOff();
    this->ValveOff(this->RelVal);
}
void Controller::FreeWalk(){
    if(!this->freeWalk_on){
        this->FreeWalk_on();
        this->freeWalk_on = true;
    }
    else{
        this->FreeWalk_off();
        this->freeWalk_on = false;
    }
}

FSMachine::FSMachine(/* args */)
{
}

FSMachine::~FSMachine()
{
}

#define RHIP_PREP_POS 450
#define KNE_SUP_PRE 200
#define ANK_ACT_PRE 300

char FSMachine::CalState(int *curMea,char curState){
    
    

    char nextState;
    if(curState ==Phase1){
        if(true){
            nextState = Phase1;
        }
        else
        {
            nextState = Phase2;
        }
    }
    else if(curState == Phase2){
        

    }
    else if(curState==Phase3){

    }
    else if(curState == Phase4){

    }
    else if(curState==Phase5){

    }
    else if(curState==Phase6){

    }
    else if(curState == Phase7){

    }
    else if(curState==Phase8){

    }
  
    return nextState;
}
void Controller::Init_swing(char side){
    if(side == 'r'){
        this->ValveOff(this->RKneVal);
        this->SetDuty(this->RKnePreVal,0);
        this->ValveOn(this->RBalVal);
    }
    else{
        this->ValveOff(this->LKneVal);
        this->SetDuty(this->LKnePreVal,0);
        this->ValveOn(this->LBalVal);
    }
}
void Controller::Mid_swing(char side){
    if(side =='r'){
        this->ValveOff(this->RBalVal);
        this->ValveOff(this->RKneVal);
    }
    else{
        this->ValveOff(this->LBalVal);
        this->ValveOff(this->LKneVal);

    }
}
void Controller::Term_swing(char side){
    if(side == 'r'){
        this->ValveOff(this->RKneVal);
        this->CheckSupPre(this->RKnePreVal,KNE_SUP_PRE);

    }
    else{
        this->ValveOff(this->LKneVal);
        this->CheckSupPre(this->LKnePreVal,KNE_SUP_PRE);
    }
}
void Controller::Load_resp(char side){
    if(side=='r'){
        this->ValveOff(this->RKneVal);
        this->KnePreRec(this->RKnePreVal,this->senData[RKNEPRE],this->senData[TANKPRE]);
        this->AnkPreRec(this->RAnkPreVal,this->senData[RANKPRE],this->senData[TANKPRE],this->RBalVal);
        
    }
    else{
        this->ValveOff(this->LKneVal);
        this->KnePreRec(this->LKnePreVal,this->senData[LKNEPRE],this->senData[TANKPRE]);
        this->AnkPreRec(this->LAnkPreVal,this->senData[LANKPRE],this->senData[TANKPRE],this->LBalVal);
        
    }
}
void Controller::Mid_stance(char side){
    // do nothing 


}
void Controller::Term_stance(char side){
    if(side=='r'){
        this->AnkPreRec(this->RAnkPreVal,this->senData[RANKPRE],this->senData[TANKPRE],this->RBalVal);
    }
    else{
        this->AnkPreRec(this->LAnkPreVal,this->senData[LANKPRE],this->senData[TANKPRE],this->LBalVal);
    }

}
void Controller::Pre_swing(char side){
    // do nothing
    if(side=='r'){
        this->ValveOn(this->RBalVal);
        this->AnkPushOff(this->RAnkPreVal,ANK_ACT_PRE);
    }
    else{
        this->ValveOn(this->LBalVal);
        this->AnkPushOff(this->LAnkPreVal,ANK_ACT_PRE);

    }
}

void Controller::BipedEngRec(){
    char curPhase = this->FSM.CalState(this->senData,this->curState);
    switch (curPhase){
        case Phase1:
            this->Load_resp('l');
            this->Init_swing('r');
            break;
        case Phase2:
            this->Mid_stance('l');
            this->Mid_swing('r');
            break;
        case Phase3:
            this->Term_stance('l');
            break;
        case Phase4:
            this->Pre_swing('l');
            this->Term_swing('r');
            break;
        case Phase5:
            this->Init_swing('l');
            this->Load_resp('r');
            break;
        case Phase6:
            this->Mid_swing('l');
            this->Mid_stance('r');
            break;
        case Phase7:
            this->Term_stance('r');
            break;
        case Phase8:
            this->Term_swing('l');
            this->Pre_swing('r');
            break;
        
    }
}
void Controller::KnePreRec(std::shared_ptr<PWMGen> knePreVal,int knePre,int tankPre){
    if(knePre-tankPre>10){

    }
    else{
        
    }

}
void Controller::AnkPreRec(std::shared_ptr<PWMGen> ankPreVal,int ankPre,int tankPre,std::shared_ptr<Valve> balVal){
    //When recycle ankle pressure, if the pressure is too high, we direct the ankle pressure to the knee joint
    if(ankPre-tankPre>10){

    }
    else{
        
        this->ValveOff(balVal);

    }

}
void Controller::CheckSupPre(std::shared_ptr<PWMGen> preVal,int supPre){


}
void Controller::AnkPushOff(std::shared_ptr<PWMGen> ankPreVal,int actPre){

}
void Controller::TestRAnk(){
    if(this->TestRAnkFlag){
        this->SetDuty(this->RAnkPreVal,0);
        this->TestRAnkFlag = false;
        this->ValveOff(this->RBalVal);
        this->ValveOn(this->RKneVal);
    }
    else{
        this->ValveOff(this->RKneVal);
        this->ValveOn(this->RBalVal);
        this->TestRAnkFlag = true;
        this->SetDuty(this->RAnkPreVal,100);

    }
}
void Controller::TestLAnk(){
    if(this->TestLAnkFlag){
        std::cout<<"turn off LAnk\n";
        this->TestLAnkFlag = false;
        this->SetDuty(this->LAnkPreVal,0);
        this->ValveOff(this->LBalVal);
        this->ValveOff(this->LKneVal);
    }
    else{
        std::cout<<"turn on LAnk\n";
        this->ValveOn(this->LKneVal);
        this->ValveOn(this->LBalVal);
        this->TestLAnkFlag = true;
        this->SetDuty(this->LAnkPreVal,100);
    }
}
void Controller::ShowSen(){
    std::cout<<"\nCurrent Sen: "<<std::setw(8)<<this->senData[0];
    for(int i=1;i<NUMSEN+1;i++){
        std::cout<<","<<std::setw(4)<<this->senData[i];
    }
    std::cout<<std::endl;

}