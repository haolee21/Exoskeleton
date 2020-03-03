#include "Controller.h"

typedef std::chrono::duration<long, std::nano> nanosecs_t;
typedef std::chrono::duration<int, std::micro> microsecs_t;
typedef std::chrono::duration<int, std::milli> millisecs_t;

//==================================================================================================================
Controller::Controller(std::string filePath, Com *_com, bool _display, std::chrono::system_clock::time_point _origin, long _sampT)
{
    memset(this->curComArray, false, sizeof(bool) * NUMCOM);

    this->sampT = _sampT;
    this->senData[0] = 0;
    this->preSen[0] = 0;
    this->testSendCount = 0;
    this->preSend = true;
    this->display = _display;
    if (_display)
    {
        this->client.reset(new Displayer());
        // this->client.reset(new Displayer());
    }

    this->valveCond.reset(new char[VALNUM]);
    this->pwmDuty.reset(new char[PWMNUM]);

    this->com = _com;

    this->LKneVal.reset(new Valve("LKneVal", filePath, OP13, 0));
    this->RKneVal.reset(new Valve("RKneVal", filePath, OP8, 1));
    this->LAnkVal.reset(new Valve("LAnkVal", filePath, OP6, 2));
    this->RAnkVal.reset(new Valve("RAnkVal", filePath, OP9, 3));
    this->LBalVal.reset(new Valve("LBalVal", filePath, OP7, 4));
    this->RBalVal.reset(new Valve("RBalVal", filePath, OP10, 5));

    this->RelVal.reset(new Valve("RelVal", filePath, OP16, 6));

    //this is useless now since I use this pin to reset arduino
    this->trParam.testOut.reset(new Valve("SyncOut", filePath, SYNCOUT, 7)); //this is not a valve, the valve index is meaningless, we do not count it in VALNUM
    // this->KnePreVal = new PWMGen("KnePreVal",filePath,OP1,30000L);
    // this->AnkPreVal = new PWMGen("AnkPreVal",filePath,OP2,30000L);
    this->LKnePreVal.reset(new PWMGen("LKnePreVal", filePath, OP1, 40000L, 0, _origin));
    this->LAnkPreVal.reset(new PWMGen("LAnkPreVal", filePath, OP2, 40000L, 1, _origin));
    this->RKnePreVal.reset(new PWMGen("RKnePreVal", filePath, OP3, 40000L, 2, _origin));
    this->RAnkPreVal.reset(new PWMGen("RAnkPreVal", filePath, OP4, 40000L, 3, _origin));

    this->PWMList[0] = this->LKnePreVal;
    this->PWMList[1] = this->LAnkPreVal;
    this->PWMList[2] = this->RKnePreVal;
    this->PWMList[3] = this->RAnkPreVal;

    this->ValveList[0] = this->LKneVal;
    this->ValveList[1] = this->RKneVal;
    this->ValveList[2] = this->LAnkVal;
    this->ValveList[3] = this->RAnkVal;
    this->ValveList[4] = this->LBalVal;
    this->ValveList[5] = this->RBalVal;
    this->ValveList[6] = this->RelVal;

    std::shared_ptr<PWMGen> *begPWM = this->PWMList;
    do
    {
        this->SetDuty(*begPWM, 0);
        (*begPWM)->Start();
    } while (++begPWM != std::end(this->PWMList));

    std::shared_ptr<Valve> *begVal = this->ValveList;
    do
    {
        this->ValveOff(*begVal);
    } while (++begVal != std::end(this->ValveList));

    // this->trParam.testOut.reset(new Valve("TestMea", filePath, 8, 6)); //this uses gpio2
    // this->ValveOff(this->trParam.testOut);
    this->curState = 0;


    auto p1_fun = [this]() {
        this->Load_resp('l');
    };
    auto p2_fun = [this]() {
        this->Mid_stance('l');
        this->Pre_swing('r');
    };
    auto p3_fun = [this]() {
        this->Init_swing('r');
    };
    auto p4_fun = [this]() {
        this->Mid_swing('r');
    };
    auto p5_fun = [this]() {
        this->Term_stance('l');
        this->Term_swing('r');
    };
    auto p6_fun = [this]() {
        this->Load_resp('r'); 
    };
    auto p7_fun = [this]() {
        this->Pre_swing('l');
        this->Mid_stance('r');
    };
    auto p8_fun = [this]() {
        this->Init_swing('l');
    };
    auto p9_fun = [this]() {
        this->Mid_swing('l');
    };
    auto p10_fun = [this]() {
        this->Term_swing('l');
        this->Term_stance('r');
    };

    this->FSM.reset(new FSMachine(filePath,p1_fun,p2_fun,p3_fun,p4_fun,p5_fun,p6_fun,p7_fun,p8_fun,p9_fun,p10_fun));
    // now we need to actuate some valves, since we prefer the system to isolate each chambers
    this->ValveOn(this->LKneVal);
    this->ValveOn(this->RKneVal);
    this->ValveOn(this->LBalVal);
    this->ValveOn(this->RBalVal);



    
}
Controller::~Controller()
{
    std::cout << "destory controller\n";
    // this->FSM_stop();
    this->PreRel();
    std::shared_ptr<PWMGen> *begPWM = this->PWMList;
    do
    {

        (*begPWM)->Stop();
    } while (++begPWM != std::end(this->PWMList));

    // delete this->valveCond;
    // delete this->pwmDuty;

    //delete this->senData;
    std::cout << "controller ends\n";
}

//=================================================================================================================
void Controller::PreRel()
{
    std::cout << "Pressure Release\n";
    this->ValveOn(this->RelVal);
    sleep(RELTIME / 2);
    this->ValveOff(this->LKneVal);
    this->ValveOff(this->RKneVal);
    this->SetDuty(this->LKnePreVal, 100);
    this->SetDuty(this->RKnePreVal, 100);
    // this->ValveOn(this->RelVal);
    this->ValveOff(this->LAnkVal);
    this->ValveOff(this->RAnkVal);

    this->SetDuty(this->LAnkPreVal, 100);
    this->SetDuty(this->RAnkPreVal, 100);

    sleep(RELTIME/2);

    this->SetDuty(this->LKnePreVal, 0);
    this->SetDuty(this->LAnkPreVal, 0);
    this->SetDuty(this->RKnePreVal, 0);
    this->SetDuty(this->RAnkPreVal, 0);
    this->ValveOff(this->RelVal);
    this->LKnePreVal->SetDuty(0, senData[TIME] + RELTIME * 1000000);
    this->LAnkPreVal->SetDuty(0, senData[TIME] + RELTIME * 1000000);
    this->RKnePreVal->SetDuty(0, senData[TIME] + RELTIME * 1000000);
    this->RAnkPreVal->SetDuty(0, senData[TIME] + RELTIME * 1000000);
}
void Controller::ShutDownPWM()
{

    std::shared_ptr<PWMGen> *begPWM = this->PWMList;
    do
    {
        this->SetDuty(*begPWM, 0);
        (*begPWM)->Start();
    } while (++begPWM != std::end(this->PWMList));
}

void Controller::SendToDisp(const char *senRaw)
{

    if (this->preSend == this->dispPreScale)
    {
        char sendData[DATALEN + VALNUM + PWMNUM];
        std::copy(senRaw, senRaw + DATALEN, sendData);
        {
            std::lock_guard<std::mutex> curLock(this->ValveCondLock);
            std::copy(this->valveCond.get(), this->valveCond.get() + VALNUM, sendData + DATALEN);
            std::copy(this->pwmDuty.get(), this->pwmDuty.get() + PWMNUM, sendData + DATALEN + VALNUM);
        }

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
    if (this->tvParam.curTestCount == this->tvParam.maxTestCount)
    {
        this->tvParam.curTestCount = 0;
        if (this->tvParam.singleValCount++ < this->tvParam.maxTest)
        {
            if (this->tvParam.curValCond)
            {
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

void Controller::SetDuty(std::shared_ptr<PWMGen> pwmVal, int duty)
{
    std::lock_guard<std::mutex> curLock(this->ValveCondLock);
    pwmVal->SetDuty(duty, this->senData[TIME]);
    this->pwmDuty[pwmVal->GetIdx()] = pwmVal->duty.byte[0];
}

//============================================================================================================
void Controller::ValveOn(std::shared_ptr<Valve> val)
{
    std::lock_guard<std::mutex> curLock(this->ValveCondLock);
    val->On(this->senData[TIME]);
    this->valveCond[val->GetValIdx()] = 'd';
}

void Controller::ValveOff(std::shared_ptr<Valve> val)
{
    val->Off(this->senData[TIME]);
    this->valveCond[val->GetValIdx()] = '!';
}

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
        this->ValveOn(this->LKneVal);
        this->ValveOn(this->LAnkVal);
        this->SetDuty(this->LKnePreVal, 100);
        sleep(10);
        this->ValveOff(this->LKneVal);
        this->ValveOff(this->LAnkVal);
        this->SetDuty(this->LKnePreVal, 0);
        break;
    case 2: //test LKneeFree and LKneeSup
        std::cout << "test LKneeFree and LKneeSup\n";
        this->ValveOff(this->LKneVal);
        this->ValveOff(this->LAnkVal);
        this->SetDuty(this->LKnePreVal, 100);
        sleep(10);
        this->SetDuty(this->LKnePreVal, 0);
        break;
    case 3: //test RKneeSup
        std::cout << "test RKneeSup\n";
        this->ValveOn(this->RKneVal);
        this->ValveOn(this->RAnkVal);
        this->SetDuty(this->RKnePreVal, 100);
        sleep(10);
        this->ValveOff(this->RKneVal);
        this->ValveOff(this->RAnkVal);
        this->SetDuty(this->RKnePreVal, 0);
        break;
    case 4: //test RKneeFree and RKneeSup
        std::cout << "test RKneeFree and RKneeSup\n";
        this->ValveOff(this->RKneVal);
        this->ValveOff(this->RAnkVal);
        this->SetDuty(this->RKnePreVal, 100);
        sleep(10);
        this->SetDuty(this->RKnePreVal, 0);
        break;
    case 5: //test LAnkleSup
        std::cout << "test LAnkleSup\n";
        this->SetDuty(this->LAnkPreVal, 100);
        sleep(10);
        this->SetDuty(this->LAnkPreVal, 0);
        break;
    case 6: //test LAnkleFree and LKneeSup (looks weird but that is how the loop is connected)
        break;
    }
}
void Controller::TestAllLeakOn()
{
    std::cout << "Leak test started\n";
    this->ValveOff(this->LKneVal);
    this->ValveOff(this->RKneVal);
    this->SetDuty(this->LKnePreVal, 100);
    this->SetDuty(this->RKnePreVal, 100);
    this->ValveOff(this->LBalVal);
    this->ValveOff(this->RBalVal);
    this->ValveOff(this->LAnkVal);
    this->ValveOff(this->RAnkVal);
    this->SetDuty(this->LAnkPreVal, 100);
    this->SetDuty(this->RAnkPreVal, 100);
}
void Controller::TestAllLeakOff()
{
    this->SetDuty(this->LKnePreVal, 0);
    this->SetDuty(this->RKnePreVal, 0);
    this->SetDuty(this->LAnkPreVal, 0);
    this->SetDuty(this->RAnkPreVal, 0);
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
    this->ValveOn(this->RelVal);
}
void Controller::FreeWalk_off()
{
    this->TestAllLeakOff();
    this->ValveOff(this->RelVal);
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
        this->ValveOn(this->LBalVal);
        this->ValveOn(this->LKneVal);
        if (this->CheckSupPre_main(this->LKnePreVal, this->senData[LKNEPRE], this->senData[TANKPRE], this->kneSupPre))
        {
            std::lock_guard<std::mutex> curLock(this->com->comLock); //not necessary, but who knows one day I may use it in different thread
            this->com->comArray[PIDACTTEST] = false;
            std::cout << "done rec\n";
        }
    }
    else if (joint == 1)
    {
        this->ValveOn(this->LBalVal);
        if (this->CheckSupPre_main(this->LAnkPreVal, this->senData[LANKPRE], this->senData[TANKPRE], this->ankSupPre))
        {
            std::lock_guard<std::mutex> curLock(this->com->comLock);
            this->com->comArray[PIDACTTEST] = false;
            std::cout << "done rec\n";
        }
    }
    else if (joint == 2)
    {
        this->ValveOn(this->RBalVal);
        this->ValveOn(this->RKneVal);
        if (this->CheckSupPre_main(this->RKnePreVal, this->senData[RKNEPRE], this->senData[TANKPRE], this->kneSupPre))
        {
            std::lock_guard<std::mutex> curLock(this->com->comLock);
            this->com->comArray[PIDACTTEST] = false;
            std::cout << "done rec\n";
        }
    }
    else if (joint == 3)
    {
        this->ValveOn(this->RBalVal);
        if (this->CheckSupPre_main(this->RAnkPreVal, this->senData[RANKPRE], this->senData[TANKPRE], this->ankSupPre))
        {
            std::lock_guard<std::mutex> curLock(this->com->comLock);
            this->com->comArray[PIDACTTEST] = false;
            std::cout << "done rec\n";
        }
    }
    else
    {
        std::cout << "wrong command value\n";
    }
}
void Controller::PIDRecTest(int joint)
{

    if (joint == 0)
    {
        this->ValveOn(this->LBalVal);
        this->ValveOn(this->LKneVal);
        if (this->CheckKnePreRec_main(this->LKnePreVal, this->senData[LKNEPRE], this->senData[TANKPRE], this->kneRecPre))
        {
            std::lock_guard<std::mutex> curLock(this->com->comLock);
            this->com->comArray[CON_LKNE_REC] = false;
        }
    }
    else if (joint == 1)
    {
        this->ValveOn(this->LBalVal);
        if (this->CheckAnkPreRec_main(this->LAnkPreVal, this->senData[LANKPRE], this->senData[TANKPRE], this->LBalVal, this->ankRecPre))
        {
            std::lock_guard<std::mutex> curLock(this->com->comLock);
            this->com->comArray[CON_LANK_REC] = false;
        }
    }
    else if (joint == 2)
    {

        this->ValveOn(this->RBalVal);
        this->ValveOn(this->RKneVal);
        if (this->CheckKnePreRec_main(this->RKnePreVal, this->senData[RKNEPRE], this->senData[TANKPRE], this->kneRecPre))
        {
            std::lock_guard<std::mutex> curLock(this->com->comLock);
            this->com->comArray[CON_RKNE_REC] = false;
        }
    }
    else if (joint == 3)
    {
        this->ValveOn(this->RBalVal);
        if (this->CheckAnkPreRec_main(this->RAnkPreVal, this->senData[RANKPRE], this->senData[TANKPRE], this->RBalVal, this->ankRecPre))
        {
            std::lock_guard<std::mutex> curLock(this->com->comLock);
            this->com->comArray[CON_RANK_REC] = false;
        }
    }
    else
    {
        std::cout << "wrong command value\n";
    }
}





// some test functions
//=======================================================================================

void Controller::TestOnePWM(int pwmIdx)
{
    //kind of stupid, but need to turn on all balvals so pressure won't fill both ankle and knee
    this->ValveOn(this->LBalVal);
    this->ValveOn(this->RBalVal);

    this->SetDuty(this->PWMList[pwmIdx], 33);
    this->testOnePWM_flag = true;
}

void Controller::TestRAnk()
{
    if (this->TestRAnkFlag)
    {
        this->SetDuty(this->RAnkPreVal, 0);
        this->TestRAnkFlag = false;
        this->ValveOff(this->RBalVal);
        this->ValveOn(this->RKneVal);
    }
    else
    {
        this->ValveOff(this->RKneVal);
        this->ValveOn(this->RBalVal);
        this->TestRAnkFlag = true;
        this->SetDuty(this->RAnkPreVal, 100);
    }
}
void Controller::TestLAnk()
{
    if (this->TestLAnkFlag)
    {
        std::cout << "turn off LAnk\n";
        this->TestLAnkFlag = false;
        this->SetDuty(this->LAnkPreVal, 0);
        this->ValveOff(this->LBalVal);
        this->ValveOff(this->LKneVal);
    }
    else
    {
        std::cout << "turn on LAnk\n";
        this->ValveOn(this->LKneVal);
        this->ValveOn(this->LBalVal);
        this->TestLAnkFlag = true;
        this->SetDuty(this->LAnkPreVal, 100);
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

