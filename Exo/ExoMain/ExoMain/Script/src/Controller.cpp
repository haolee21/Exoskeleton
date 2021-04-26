#include "Controller.h"

typedef std::chrono::duration<long, std::nano> nanosecs_t;
typedef std::chrono::duration<int, std::micro> microsecs_t;
typedef std::chrono::duration<int, std::milli> millisecs_t;
using namespace std;

//==================================================================================================================
Controller::Controller(std::string filePath, Com *_com, std::chrono::system_clock::time_point _origin, long _sampT,
                        int *_encData,int *_adcData,std::mutex *_senLock)
{
    
    this->t_origin = _origin;
    this->sampT = _sampT;
    this->encData = _encData;
    this->adcData = _adcData;
    this->senLock = _senLock;
    this->com = _com;
    this->valveCon.reset(new ValveCon(filePath,_origin));
    
    // this->trParam.testOut.reset(new Valve("TestMea", filePath, 8, 6)); //this uses gpio2
    // this->ValveOff(this->trParam.testOut);
    this->curState = 0;

    auto p1_fun = [this]() {
        this->Load_resp('l');
        std::cout << "P1: left load resp\n";
    };
    auto p2_fun = [this]() {
        this->Mid_stance('l');
        this->Pre_swing('r');
        std::cout << "P2: left mid_stance, right pre_swing\n";
    };
    auto p3_fun = [this]() {
        this->Init_swing('r');
        std::cout << "P3: right init swing\n";
    };
    auto p4_fun = [this]() {
        this->Mid_swing('r');
        std::cout << "P4: right mid swing\n";
    };
    auto p5_fun = [this]() {
        this->Term_stance('l');
        this->Term_swing('r');
        std::cout << "P5: left terminal stance, right terminal swing\n";
    };
    auto p6_fun = [this]() {
        this->Load_resp('r');
        std::cout << "P6: right load resp\n";
    };
    auto p7_fun = [this]() {
        this->Pre_swing('l');
        this->Mid_stance('r');
        std::cout << "P7: left pre_swing, right mid_stance\n";

    };
    auto p8_fun = [this]() {
        this->Init_swing('l');
        std::cout<<"P8: left init_swing\n";
    };
    auto p9_fun = [this]() {
        this->Mid_swing('l');
        std::cout<<"P9: left mid_swing\n";
    };
    auto p10_fun = [this]() {
        this->Term_swing('l');
        this->Term_stance('r');
        std::cout<<"P10: left term_swing, right term_stance\n";
    };

    this->FSM.reset(new FSMachine(filePath,p1_fun,p2_fun,p3_fun,p4_fun,p5_fun,p6_fun,p7_fun,p8_fun,p9_fun,p10_fun));
    // now we need to actuate some valves, since we prefer the system to isolate each chambers
    this->valveCon->setVal(LKNE_VAL,ValveCon::ValSwitch::On);
    this->valveCon->setVal(RKNE_VAL,ValveCon::ValSwitch::On);
    this->valveCon->setVal(LBAL_VAL,ValveCon::ValSwitch::On);
    this->valveCon->setVal(RBAL_VAL,ValveCon::ValSwitch::On);
    
}
Controller::~Controller()
{
    std::cout << "destory controller\n";
    // this->FSM_stop();
    this->PreRel();
    this->valveCon->pwm_all_off();


    std::cout << "controller ends\n";
}

//=================================================================================================================
void Controller::PreRel()
{
    auto elapsed =  std::chrono::high_resolution_clock::now() - this->t_origin;
    long long curTime = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
    std::cout << "Pressure Release\n";
    this->valveCon->setVal(PREL_VAL,ValveCon::ValSwitch::On);
    sleep(RELTIME / 2);

    elapsed =  std::chrono::high_resolution_clock::now() - this->t_origin;
    curTime = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
    this->valveCon->setVal(LKNE_VAL,ValveCon::ValSwitch::Off);
    this->valveCon->setVal(RKNE_VAL,ValveCon::ValSwitch::Off);
    this->valveCon->setDuty(LKNE_PWM,100);
    this->valveCon->setDuty(RKNE_PWM,100);
    
    this->valveCon->setVal(LANK_VAL,ValveCon::ValSwitch::Off);
    this->valveCon->setVal(RANK_VAL,ValveCon::ValSwitch::Off);
    
    this->valveCon->setDuty(LANK_PWM,100);
    this->valveCon->setDuty(RANK_PWM,100);
    sleep(RELTIME/2);
    elapsed =  std::chrono::high_resolution_clock::now() - this->t_origin;
    curTime = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();


    this->valveCon->setDuty(LKNE_PWM,0);
    this->valveCon->setDuty(LANK_PWM,0);
    this->valveCon->setDuty(RKNE_PWM,0);
    this->valveCon->setDuty(RKNE_PWM,0);
    this->valveCon->setDuty(RANK_PWM,0);
    this->valveCon->setVal(PREL_VAL,ValveCon::ValSwitch::Off);
    this->valveCon->setDuty(LKNE_PWM,0);
    this->valveCon->setDuty(LANK_PWM,0);
    this->valveCon->setDuty(RKNE_PWM,0);
    this->valveCon->setDuty(RANK_PWM,0);

}
void Controller::ShutDownPWM()
{

    this->valveCon->pwm_all_off();
}


//Test Valve
//============================================================================================================
void Controller::TestValve()
{
    if (this->tvParam.curTestCount == this->tvParam.maxTestCount)
    {
        this->tvParam.curTestCount = 0;
        if (this->tvParam.singleValCount++ < this->tvParam.maxTest)
        {
            if (this->tvParam.curValCond)
            {   
                this->valveCon->setVal()
                this->ValveOn(this->ValveList[this->tvParam.testValIdx]);
                this->tvParam.curValCond = false;
            }
            else
            {
                this->ValveOff(this->ValveList[this->tvParam.testValIdx]);
                this->tvParam.curValCond = true;
            }
        }
        else
        {
            this->tvParam.singleValCount = 0;
            this->tvParam.testValIdx++;
            if (this->tvParam.testValIdx == VALNUM)
            {
                this->tvParam.testValIdx = 0;
            }
        }
    }
    else
    {
        this->tvParam.curTestCount++;
    }
}
//============================================================================================================


// PWM
//============================================================================================================
void Controller::TestPWM()
{
    if (this->tpParam.dutyLoopCount == this->tpParam.preScaler)
    {
        this->tpParam.dutyLoopCount = 0;
        if (this->tpParam.curTestDuty == 100)
        {
            this->tpParam.curTestDuty = 0;
            this->tpParam.testPWMidx++;
            if (this->tpParam.testPWMidx == PWMNUM)
                this->tpParam.testPWMidx = 0;
        }
        else
            this->tpParam.curTestDuty += 10;
        this->SetDuty(this->PWMList[this->tpParam.testPWMidx], this->tpParam.curTestDuty);
    }
    else
    {
        this->tpParam.dutyLoopCount++;
    }
}


//============================================================================================================

int Controller::CalDuty(int curPre, int desPre, int tankPre)
{
    if (curPre > desPre)
        return 0;
    else
    {
        if (tankPre > curPre)
        {
            if (tankPre > desPre)
            {
                int duty = (desPre - curPre) * 100 / (tankPre - curPre);
                std::cout << "duty need: " << duty << std::endl;
                return duty;
            }
            else
                return 0;
        }
        else
            return 0;
    }
}

void Controller::SampKneMod(int testDuty)
{
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
void Controller::KneRel()
{
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
void Controller::TestLeak(int curPath)
{
    switch (curPath)
    {
    case 0: //test LKneeSup
        std::cout << "test LKneeSup\n";
        this->valveCon->setVal(LKNE_VAL,ValveCon::ValSwitch::On);
        this->valveCon->setVal(LANK_VAL,ValveCon::ValSwitch::On);
        this->valveCon->setDuty(LKNE_PWM,100);
        sleep(10);
        this->valveCon->setVal(LKNE_VAL,ValveCon::ValSwitch::Off);
        this->valveCon->setVal(LANK_VAL,ValveCon::ValSwitch::Off);
        this->valveCon->setVal(LKNE_PWM,0);
        
        break;
    case 2: //test LKneeFree and LKneeSup
        std::cout << "test LKneeFree and LKneeSup\n";
        this->valveCon->setVal(LKNE_VAL,ValveCon::ValSwitch::Off);
        this->valveCon->setVal(LANK_VAL,ValveCon::ValSwitch::Off);
        this->valveCon->setDuty(LKNE_PWM,100);
        
        sleep(10);
        this->valveCon->setDuty(LKNE_PWM,0);
        break;
    case 3: //test RKneeSup
        std::cout << "test RKneeSup\n";
        this->valveCon->setVal(RKNE_VAL,ValveCon::ValSwitch::On);
        this->valveCon->setVal(RKNE_VAL,ValveCon::ValSwitch::On);
        this->valveCon->setVal(RANK_VAL,ValveCon::ValSwitch::On);
        this->valveCon->setVal(RANK_VAL,ValveCon::ValSwitch::On);
        this->valveCon->setDuty(RKNE_PWM,100);
        
        sleep(10);
        this->valveCon->setVal(RKNE_VAL,ValveCon::ValSwitch::Off);
        this->valveCon->setVal(RANK_VAL,ValveCon::ValSwitch::Off);
        this->valveCon->setDuty(RKNE_PWM,0);
        break;
    case 4: //test RKneeFree and RKneeSup
        std::cout << "test RKneeFree and RKneeSup\n";
        this->valveCon->setVal(RKNE_VAL,ValveCon::ValSwitch::Off);
        this->valveCon->setVal(RANK_VAL,ValveCon::ValSwitch::Off);
        this->valveCon->setDuty(RKNE_PWM,100);
        sleep(10);
        this->valveCon->setDuty(RKNE_PWM,0);
        
        break;
    case 5: //test LAnkleSup
        std::cout << "test LAnkleSup\n";
        this->valveCon->setDuty(LANK_PWM,100);
        
        sleep(10);
        this->valveCon->setDuty(LANK_PWM,0);
        
        break;
    case 6: //test LAnkleFree and LKneeSup (looks weird but that is how the loop is connected)
        break;
    }
}
void Controller::TestAllLeakOn()
{
    std::cout << "Leak test started\n";
    this->valveCon->setVal(LKNE_VAL,ValveCon::ValSwitch::Off);
    this->valveCon->setVal(RKNE_VAL,ValveCon::ValSwitch::Off);
    this->valveCon->setDuty(LKNE_PWM,100);
    this->valveCon->setDuty(RKNE_PWM,100);
    this->valveCon->setVal(LBAL_VAL,ValveCon::ValSwitch::Off);
    this->valveCon->setVal(RBAL_VAL,ValveCon::ValSwitch::Off);
    this->valveCon->setVal(LANK_VAL,ValveCon::ValSwitch::Off);
    this->valveCon->setVal(RANK_VAL,ValveCon::ValSwitch::Off);
    this->valveCon->setDuty(LANK_PWM,100);
    this->valveCon->setDuty(RANK_PWM,100);
    
}
void Controller::TestAllLeakOff()
{
    this->valveCon->setDuty(LKNE_PWM,0);
    this->valveCon->setDuty(RKNE_PWM,0);
    this->valveCon->setDuty(LANK_PWM,0);
    this->valveCon->setDuty(RANK_PWM,0);
    
    this->com->comArray[TESTALLLEAK] = false;
    std::cout << "leak test ends\n";
}
void Controller::TestAllLeak()
{
    if (this->testLeak.all_on)
    {
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
void Controller::FreeWalk_on()
{
    //call TestAllLeak with RelVal on

    this->TestAllLeakOn();
    this->valveCon->setVal(PREL_VAL,ValveCon::ValSwitch::On);
}
void Controller::FreeWalk_off()
{
    this->TestAllLeakOff();
    this->valveCon->setVal(PREL_VAL,ValveCon::ValSwitch::Off);
    
}
void Controller::FreeWalk()
{
    if (!this->freeWalk_on)
    {
        this->FreeWalk_on();
        this->freeWalk_on = true;
    }
    else
    {
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
void Controller::PIDActTest(int joint)
{
    //although it is called act test, for knee support, it is also an action
    if (joint == 0)
    {
        this->valveCon->setVal(LBAL_VAL,ValveCon::ValSwitch::On);
        this->valveCon->setVal(LKNE_VAL,ValveCon::ValSwitch::On);
        
        if (this->CheckSupPre_main(this->LKnePreVal, this->senData[LKNEPRE], this->senData[TANKPRE], this->kneSupPre))
        {    
            this->com->comArray[PIDACTTEST] = false;
            std::cout << "done rec\n";
        }
    }
    if (joint == 1)
    {
        this->valveCon->setVal(LBAL_VAL,ValveCon::ValSwitch::On);
        if (this->CheckSupPre_main(this->LAnkPreVal, this->senData[LANKPRE], this->senData[TANKPRE], this->ankSupPre))
        {
            
            this->com->comArray[PIDACTTEST] = false;
            std::cout << "done rec\n";
        }
    }
    if (joint == 2)
    {
        this->valveCon->setVal(RBAL_VAL,ValveCon::ValSwitch::On);
        this->valveCon->setVal(RKNE_VAL,ValveCon::ValSwitch::On);
        
        if (this->CheckSupPre_main(this->RKnePreVal, this->senData[RKNEPRE], this->senData[TANKPRE], this->kneSupPre))
        {
            
            this->com->comArray[PIDACTTEST] = false;
            std::cout << "done rec\n";
        }
    }
    if (joint == 3)
    {
        this->valveCon->setVal(RBAL_VAL,ValveCon::ValSwitch::On);
        
        if (this->CheckSupPre_main(this->RAnkPreVal, this->senData[RANKPRE], this->senData[TANKPRE], this->ankSupPre))
        {
            
            this->com->comArray[PIDACTTEST] = false;
            std::cout << "done rec\n";
        }
    }
    // else
    // {
    //     std::cout << "wrong command value\n";
    // }
}
void Controller::PIDRecTest(int joint)
{

    if (joint == 0)
    {
        this->valveCon->setVal(LBAL_VAL,ValveCon::ValSwitch::On);
        this->valveCon->setVal(LKNE_VAL,ValveCon::ValSwitch::On);
        
        if (this->CheckKnePreRec_main(this->LKnePreVal, this->senData[LKNEPRE], this->senData[TANKPRE], this->kneRecPre))
        {
            
            this->com->comArray[CON_LKNE_REC] = false;
        }
    }
    if (joint == 1)
    {
        this->valveCon->setVal(LBAL_VAL,ValveCon::ValSwitch::On);
        if (this->CheckAnkPreRec_main(this->LAnkPreVal, this->senData[LANKPRE], this->senData[TANKPRE], this->LBalVal, this->ankRecPre))
        {
            
            this->com->comArray[CON_LANK_REC] = false;
        }
    }
    if (joint == 2)
    {
        this->valveCon->setVal(RBAL_VAL,ValveCon::ValSwitch::On);
        this->valveCon->setVal(RKNE_VAL,ValveCon::ValSwitch::On);        
        if (this->CheckKnePreRec_main(this->RKnePreVal, this->senData[RKNEPRE], this->senData[TANKPRE], this->kneRecPre))
        {
            
            this->com->comArray[CON_RKNE_REC] = false;
        }
    }
    if (joint == 3)
    {
        this->valveCon->setVal(RBAL_VAL,ValveCon::ValSwitch::On);
        if (this->CheckAnkPreRec_main(this->RAnkPreVal, this->senData[RANKPRE], this->senData[TANKPRE], this->RBalVal, this->ankRecPre))
        {
            
            this->com->comArray[CON_RANK_REC] = false;
        }
    }
    // else
    // {
    //     std::cout << joint<<" wrong command value\n";
    // }
}





// some test functions
//=======================================================================================

void Controller::TestOnePWM(int pwmIdx)
{
    //kind of stupid, but need to turn on all balvals so pressure won't fill both ankle and knee
    this->valveCon->setVal(LBAL_VAL,ValveCon::ValSwitch::On);
    this->valveCon->setVal(RBAL_VAL,ValveCon::ValSwitch::On);
    this->valveCon->setDuty(pwmIdx,33);
    this->testOnePWM_flag = true;
}

void Controller::TestRAnk()
{
    if (this->TestRAnkFlag)
    {
        this->valveCon->setDuty(RKNE_PWM,0);
        this->TestRAnkFlag = false;
        this->valveCon->setVal(RBAL_VAL,ValveCon::ValSwitch::Off);
        this->valveCon->setVal(RKNE_VAL,ValveCon::ValSwitch::On);
       
    }
    else
    {
        this->valveCon->setVal(RKNE_VAL,ValveCon::ValSwitch::Off);
        this->valveCon->setVal(RBAL_VAL,ValveCon::ValSwitch::On);
        
        this->TestRAnkFlag = true;
        this->valveCon->setDuty(RANK_PWM,100);
        
    }
}
void Controller::TestLAnk()
{
    if (this->TestLAnkFlag)
    {
        std::cout << "turn off LAnk\n";
        this->TestLAnkFlag = false;
        this->valveCon->setDuty(LANK_PWM,0);
        this->valveCon->setVal(LBAL_VAL,ValveCon::ValSwitch::Off);
        this->valveCon->setVal(LKNE_VAL,ValveCon::ValSwitch::Off);
        
    }
    else
    {
        std::cout << "turn on LAnk\n";
        this->valveCon->setVal(LKNE_VAL,ValveCon::ValSwitch::On);
        this->valveCon->setVal(LBAL_VAL,ValveCon::ValSwitch::On);
        
        this->TestLAnkFlag = true;
        
        this->valveCon->setDuty(LANK_PWM,100);
    }
}
void Controller::ShowSen()
{
    std::cout << "\nCurrent Sen: " << std::setw(8) << this->senData[0];
    for (int i = 1; i < NUMSEN + 1; i++)
    {
        std::cout << "," << std::setw(4) << this->senData[i];
    }
    std::cout << std::endl;
}
void Controller::Start(int *senData, char *senRaw, std::mutex *senDataLock)
{
    if (!this->sw_conMain)
    {
        this->conMain_th = std::thread(&Controller::ConMainLoop, this, senData, senRaw, senDataLock);
        this->sw_conMain = true;
    }
    else
    {
        std::cout << "controller already started\n";
    }
}
void Controller::Stop()
{
    bool conCond;
    {
        std::lock_guard<std::mutex> curLock(this->conSW_lock);
        conCond = this->sw_conMain;
    }
    if (conCond)
    {
        {
            std::lock_guard<std::mutex> curLock(this->conSW_lock);
            this->sw_conMain = false;
        }

        this->conMain_th.join();
    }
    std::cout << "Controller stop\n";
   
}

