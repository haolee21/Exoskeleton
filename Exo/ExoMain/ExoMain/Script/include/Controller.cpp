#include "Controller.h"




typedef std::chrono::duration<long, std::nano> nanosecs_t;
typedef std::chrono::duration<int, std::micro> microsecs_t;
typedef std::chrono::duration<int, std::milli> millisecs_t;

//==================================================================================================================
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


    // now we need to actuate some valves, since we prefer the system to isolate each chambers
    this->ValveOn(this->LKneVal);
    this->ValveOn(this->RKneVal);
    this->ValveOn(this->LBalVal);
    this->ValveOn(this->RBalVal);



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

//=================================================================================================================
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
void Controller::ShutDownPWM(){

    std::shared_ptr<PWMGen> *begPWM=this->PWMList;
    do{
        this->SetDuty(*begPWM,0);
        (*begPWM)->Start();
    }while(++begPWM!=std::end(this->PWMList));
    

}
// main loop
// =================================================================================================================
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
        if(this->com->comArray[PIDACTTEST]){
            taskQue.push(std::thread(&Controller::PIDActTest,this,this->com->comVal[PIDACTTEST]));
        }
        if(this->com->comArray[TESTONEPWM]){
            if(this->testOnePWM_flag){
                this->SetDuty(this->PWMList[com->comVal[TESTONEPWM]],0); //this is a bit hack, I need to turn off pwm valves
                cout<<"turn off\n";
                
                this->testOnePWM_flag=false;
            }
            else{
                taskQue.push(std::thread(&Controller::TestOnePWM,this,this->com->comVal[TESTONEPWM]));
                this->testOnePWM_flag=true;
            }
            this->com->comArray[TESTONEPWM] = false;
            
        }
    }
    if (this->display)
    {
        taskQue.push(std::thread(&Controller::SendToDisp,this,senRaw));
    }
    while (!taskQue.empty())
    {
        taskQue.front().join();
        taskQue.pop();
    }
    
    std::memcpy(this->preSen,this->senData,std::size_t((NUMSEN+1)*sizeof(int)));
    
}
void Controller::SendToDisp(const char *senRaw){
    if(this->preSend==this->dispPreScale){
            char sendData[DATALEN + VALNUM + PWMNUM];
            std::copy(senRaw, senRaw + DATALEN, sendData);
            std::copy(this->valveCond, this->valveCond + VALNUM, sendData + DATALEN);
            std::copy(this->pwmDuty, this->pwmDuty + PWMNUM, sendData + DATALEN + VALNUM);
            this->client->send(sendData, DATALEN + VALNUM + PWMNUM);
            this->preSend = 0;

    }
    else
    {
        this->preSend++;
        //     this->preSend = true;
    }

}
//Test Valve 
//============================================================================================================
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
//============================================================================================================
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

// PWM
//============================================================================================================
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


void Controller::SetDuty(std::shared_ptr<PWMGen> pwmVal, int duty)
{
    pwmVal->SetDuty(duty, this->senData[TIME]);
    this->pwmDuty[pwmVal->GetIdx()] = pwmVal->duty.byte[0];
}


//============================================================================================================
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

//==============================================================================================================
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

//=================================================================================================================
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
// PID controller test
// we test the PID controller here, check if it is possible to reach desired pressure/ recycle to desire pressure
// since the command value is defined as int, we classfied it into 4 categories
// 0: left knee
// 1: left ankle
// 2: right knee
// 3: right ankle
//===================================================================================================================
void Controller::PIDActTest(int joint){
    //although it is called act test, for knee support, it is also an action
    if(joint==0){
        this->ValveOn(this->LBalVal);
        if(this->CheckSupPre(this->LKnePreVal,this->senData[LKNEPRE],this->senData[TANKPRE])){
            this->com->comArray[PIDACTTEST] = false;
        }
    }     
    else if (joint ==1){
        this->ValveOn(this->LBalVal);
        if(this->CheckSupPre(this->LAnkPreVal,this->senData[LANKPRE],this->senData[TANKPRE])){
            this->com->comArray[PIDACTTEST] = false;
        }
        
    }
    else if(joint ==2){
        this->ValveOn(this->RBalVal);
        if(this->CheckSupPre(this->RKnePreVal,this->senData[RKNEPRE],this->senData[TANKPRE])){
            this->com->comArray[PIDACTTEST] = false;
        }
    }
    else if(joint ==3){
        this->ValveOn(this->RBalVal);
        if(this->CheckSupPre(this->RAnkPreVal,this->senData[RANKPRE],this->senData[TANKPRE])){
            this->com->comArray[PIDACTTEST] = false;
        }

    }
    else{
        std::cout<<"wrong command value\n";
    }
    
}
void Controller::PIDRecTest(int desPre,int joint){
    if(joint==0){
    }     
    else if (joint ==1){
    }
    else if(joint ==2){

    }
    else if(joint ==3){

    }
    else{
        std::cout<<"wrong command value\n";
    }
    


}

// Data pre-process for the PID controller (feedback linearization?)
//===========================================================================================
float Controller::SupPreInput(int knePre,int tankPre){

    return (float)(this->kneSupPre-knePre)/(tankPre-knePre);
}
float Controller::AnkActInput(int ankPre,int tankPre){
    return (float)(this->ankActPre-ankPre)/(tankPre-ankPre);
}
float Controller::AnkPreRecInput(int ankPre,int tankPre){
    return (float)ankPre/tankPre;
}
float Controller::KnePreRecInput(int knePre,int tankPre){
    return (float)knePre/tankPre;
}





// Finite state machine 
//===================================================================================================================

void Controller::BipedEngRec(){
    this->curState = this->FSM.CalState(this->senData,this->curState);
    switch (this->curState){
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


// 7 Phases of single leg's walking gait
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
        this->CheckSupPre(this->RKnePreVal,this->senData[RKNEPRE],this->senData[TANKPRE]);

    }
    else{
        this->ValveOff(this->LKneVal);
        this->CheckSupPre(this->LKnePreVal,this->senData[LKNEPRE],this->senData[TANKPRE]);
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
    // in terminal stance, the ankle need to slow down while knee needs to extend, so we use balance valve to achieve it.
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
        this->AnkPushOff(this->RAnkPreVal,this->senData[RANKPRE],this->senData[TANKPRE]);
    }
    else{
        this->ValveOn(this->LBalVal);
        this->AnkPushOff(this->LAnkPreVal,this->senData[LANKPRE],this->senData[TANKPRE]);

    }
}


// Actions
// detail of how pressure valve will operate is defined here
//===========================================================================================
void Controller::KnePreRec(std::shared_ptr<PWMGen> knePreVal,int knePre,int tankPre){
    if(knePre-tankPre>10){
        if(!this->kneRecPID){
            this->kneRecPID.reset(new PIDCon(100,0.01,0.001,this->KnePreRecInput(knePre,tankPre)));
        }
        else
            this->SetDuty(knePreVal,this->kneRecPID->GetDuty(this->KnePreRecInput(knePre,tankPre),this->senData[TIME]));

    }
    else{
        this->SetDuty(knePreVal,0);
        this->kneRecPID.reset();
        
    }

}

void Controller::AnkPreRec(std::shared_ptr<PWMGen> ankPreVal,int ankPre,int tankPre,std::shared_ptr<Valve> balVal){
    //When recycle ankle pressure, if the pressure is too high, we direct the ankle pressure to the knee joint
    if(ankPre-tankPre>10){
        if(!this->ankRecPID){
            this->ankRecPID.reset(new PIDCon(100,0.1,0.001,this->AnkPreRecInput(ankPre,tankPre)));
        }
        this->ValveOn(balVal);
        this->SetDuty(ankPreVal,this->ankRecPID->GetDuty(this->AnkPreRecInput(ankPre,tankPre),this->senData[TIME]));
    }
    else{
        
        this->ValveOff(balVal);
        this->ankRecPID.reset();
    }
}

bool Controller::CheckSupPre(std::shared_ptr<PWMGen> preVal,int knePre,int tankPre){
    if(knePre-this->kneSupPre<10){
        if(!this->kneSupPID){
            this->kneSupPID.reset(new PIDCon(100,0.1,0.01,this->SupPreInput(knePre,tankPre)));
        }
        else{
            this->SetDuty(preVal,this->kneSupPID->GetDuty(this->SupPreInput(knePre,tankPre),this->senData[TIME]));
        }
        return false;
    }
    else{
        std::cout<<"done\n";
        this->SetDuty(preVal,0);
        this->kneSupPID.reset();
        return true;
    }
    

}
void Controller::AnkPushOff(std::shared_ptr<PWMGen> ankPreVal,int ankPre,int tankPre){
    if(this->ankActPre-ankPre<10){
        if(!this->ankActPID){
        this->ankActPID.reset(new PIDCon(800,0.01,0.001,this->AnkActInput(ankPre,tankPre)));
        }
        else{
            this->SetDuty(ankPreVal,this->ankActPID->GetDuty(this->AnkActInput(ankPre,tankPre),this->senData[TIME]));
        }

    }
    else{
        this->ankActPID.reset();
        this->SetDuty(ankPreVal,0);
    }

}



// some test functions
//=======================================================================================

void Controller::TestOnePWM(int pwmIdx){
    //kind of stupid, but need to turn on all balvals so pressure won't fill both ankle and knee
    this->ValveOn(this->LBalVal);
    this->ValveOn(this->RBalVal);

    this->SetDuty(this->PWMList[pwmIdx],33);
    this->testOnePWM_flag = true;
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