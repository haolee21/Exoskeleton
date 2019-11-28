#include "Controller.h"

typedef std::chrono::duration<long, std::nano> nanosecs_t;
typedef std::chrono::duration<int, std::micro> microsecs_t;
typedef std::chrono::duration<int, std::milli> millisecs_t;

//==================================================================================================================
Controller::Controller(std::string filePath, Com *_com, bool _display, std::chrono::system_clock::time_point _origin,long _sampT)
{
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
    this->initGait = false;
    this->gaitStart = false;
    this->gaitEnd = true;
    this->FSM.reset(new FSMachine(filePath));

    // now we need to actuate some valves, since we prefer the system to isolate each chambers
    this->ValveOn(this->LKneVal);
    this->ValveOn(this->RKneVal);
    this->ValveOn(this->LBalVal);
    this->ValveOn(this->RBalVal);

    //FSM init
    this->p1_t = 10000l;
    this->p2_t = 10000l;
    this->p3_t = 10000l;
    this->p4_t = 10000l;
    this->p5_t = 10000l;
    this->p6_t = 10000l;
    this->p7_t = 10000l;
    this->p8_t = 10000l;
    this->p9_t = 10000l;
    this->p10_t = 10000l;
  
}
Controller::~Controller()
{
    std::cout << "destory controller\n";
    this->FSM_stop();

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
    this->ValveOff(this->LKneVal);
    this->ValveOff(this->RKneVal);
    this->SetDuty(this->LKnePreVal, 100);
    this->SetDuty(this->RKnePreVal, 100);
    this->ValveOn(this->RelVal);
    this->ValveOff(this->LAnkVal);
    this->ValveOff(this->RAnkVal);

    this->SetDuty(this->LAnkPreVal, 100);
    this->SetDuty(this->RAnkPreVal, 100);

    sleep(RELTIME);

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
// main loop
// =================================================================================================================
void Controller::ConMainLoop(int *_senData, char *senRaw,std::mutex *senDataLock)
{

    struct timespec conTimer;
    clock_gettime(CLOCK_MONOTONIC, &conTimer);
    while (true)
    {
        
        queue<thread> taskQue;
        // this->senData = senData;
        {
            std::lock_guard<std::mutex> curLock(*senDataLock);
            std::memcpy(this->senData, _senData, std::size_t((NUMSEN + 1) * sizeof(int)));
        }
        

        {
            std::lock_guard<std::mutex> lock(this->com->comLock);
            if (this->com->comArray[TESTVAL])
                taskQue.push(std::thread(&Controller::TestValve, this));
            if (this->com->comArray[TESTPWM])
                taskQue.push(std::thread(&Controller::TestPWM, this));
            if (this->com->comArray[SHUTPWM])
            {
                taskQue.push(std::thread(&Controller::ShutDownPWM, this));
                this->com->comArray[SHUTPWM] = false;
            }

            if (this->com->comArray[KNEMODSAMP])
            {
                taskQue.push(std::thread(&Controller::SampKneMod, this, com->comVal[KNEMODSAMP]));
            }
            if (this->com->comArray[KNEPREREL])
            {
                taskQue.push(std::thread(&Controller::KneRel, this));
            }
            if (this->com->comArray[TESTALLLEAK])
            {
                taskQue.push(std::thread(&Controller::TestAllLeak, this));
                this->com->comArray[TESTALLLEAK] = false;
            }
            if (this->com->comArray[FREEWALK])
            {
                taskQue.push(std::thread(&Controller::FreeWalk, this));
                this->com->comArray[FREEWALK] = false;
            }
            if (this->com->comArray[TESTLEAK])
            {
                taskQue.push(std::thread(&Controller::TestLeak, this, com->comVal[TESTLEAK]));
                this->com->comArray[TESTLEAK] = false;
            }
            if (this->com->comArray[TESTLANK])
            {
                taskQue.push(std::thread(&Controller::TestLAnk, this));
                this->com->comArray[TESTLANK] = false;
            }
            if (this->com->comArray[TESTRANK])
            {
                taskQue.push(std::thread(&Controller::TestRAnk, this));
                this->com->comArray[TESTRANK] = false;
            }
            if (this->com->comArray[SHOWSEN])
            {
                taskQue.push(std::thread(&Controller::ShowSen, this));
                this->com->comArray[SHOWSEN] = false;
            }
            if (this->com->comArray[BIPEDREC])
            {
                taskQue.push(std::thread(&Controller::BipedEngRec, this));
            }
            if (this->com->comArray[TESTSYNC])
            {
                taskQue.push(std::thread(&Controller::TestReactingTime, this));
            }
            if (this->com->comArray[PIDACTTEST])
            {
                taskQue.push(std::thread(&Controller::PIDActTest, this, this->com->comVal[PIDACTTEST]));
            }
            if (this->com->comArray[TESTONEPWM])
            {
                if (this->testOnePWM_flag)
                {
                    this->SetDuty(this->PWMList[com->comVal[TESTONEPWM]], 0); //this is a bit hack, I need to turn off pwm valves
                    cout << "turn off\n";

                    this->testOnePWM_flag = false;
                }
                else
                {
                    taskQue.push(std::thread(&Controller::TestOnePWM, this, this->com->comVal[TESTONEPWM]));
                    this->testOnePWM_flag = true;
                }
                this->com->comArray[TESTONEPWM] = false;
            }
            if (this->com->comArray[CON_LANK_ACT])
            {
                taskQue.push(std::thread(&Controller::AnkPushOff_main, this, this->LAnkPreVal, this->senData[LANKPRE], this->senData[TANKPRE]));
            }
            if (this->com->comArray[CON_RANK_ACT])
            {
                taskQue.push(std::thread(&Controller::AnkPushOff_main, this, this->RAnkPreVal, this->senData[RANKPRE], this->senData[TANKPRE]));
            }
            if (this->com->comArray[CON_LKNE_REC])
            {
                taskQue.push(std::thread(&Controller::KnePreRec_main, this, this->LKnePreVal, this->senData[LKNEPRE], this->senData[TANKPRE], this->kneRecPre));
            }
            if (this->com->comArray[CON_RKNE_REC])
            {
                taskQue.push(std::thread(&Controller::KnePreRec_main, this, this->RKnePreVal, this->senData[RKNEPRE], this->senData[TANKPRE], this->kneRecPre));
            }
            if (this->com->comArray[CON_LANK_REC])
            {
                taskQue.push(std::thread(&Controller::AnkPreRec_main, this, this->LAnkPreVal, this->senData[LANKPRE], this->senData[TANKPRE], this->LBalVal, this->ankRecPre));
            }
            if (this->com->comArray[CON_RANK_REC])
            {
                taskQue.push(std::thread(&Controller::AnkPreRec_main, this, this->RAnkPreVal, this->senData[RANKPRE], this->senData[TANKPRE], this->RBalVal, this->ankRecPre));
            }
            if (this->com->comArray[CON_LKNE_SUP])
            {
                taskQue.push(std::thread(&Controller::CheckSupPre_main, this, this->LKnePreVal, this->senData[LKNEPRE], this->senData[TANKPRE], this->kneSupPre));
            }
            if (this->com->comArray[CON_RKNE_SUP])
            {
                taskQue.push(std::thread(&Controller::CheckSupPre_main, this, this->RKnePreVal, this->senData[RKNEPRE], this->senData[TANKPRE], this->kneSupPre));
            }
            if (this->com->comArray[CON_LANK_SUP])
            {
                taskQue.push(std::thread(&Controller::CheckSupPre_main, this, this->LAnkPreVal, this->senData[LANKPRE], this->senData[TANKPRE], this->ankSupPre));
            }
            if (this->com->comArray[CON_RANK_SUP])
            {
                taskQue.push(std::thread(&Controller::CheckSupPre_main, this, this->RAnkPreVal, this->senData[RANKPRE], this->senData[TANKPRE], this->ankSupPre));
            }
            if (this->com->comArray[CON_SET_INIT_POS])
            {
                this->com->comArray[CON_SET_INIT_POS] = false;
                taskQue.push(std::thread(&FSMachine::SetInitPos, this->FSM.get(), this->senData[LHIPPOS], this->senData[RHIPPOS]));
            }
        }
        if (this->display)
        {

            taskQue.push(std::thread(&Controller::SendToDisp, this, senRaw));
        }
        while (!taskQue.empty())
        {
            taskQue.front().join();
            taskQue.pop();
        }

        std::memcpy(this->preSen, this->senData, std::size_t((NUMSEN + 1) * sizeof(int)));
        bool cond;
        {
            std::lock_guard<std::mutex> curLock(this->conSW_lock);
            cond = this->sw_conMain;
        }
        if(!cond){
            break;
        }

        conTimer.tv_nsec += this->sampT;
        Common::tsnorm(&conTimer);
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &conTimer, NULL);
    }
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
        }
    }
    else if (joint == 1)
    {
        this->ValveOn(this->LBalVal);
        if (this->CheckSupPre_main(this->LAnkPreVal, this->senData[LANKPRE], this->senData[TANKPRE], this->ankSupPre))
        {
            this->com->comArray[PIDACTTEST] = false;
        }
    }
    else if (joint == 2)
    {
        this->ValveOn(this->RBalVal);
        this->ValveOn(this->RKneVal);
        if (this->CheckSupPre_main(this->RKnePreVal, this->senData[RKNEPRE], this->senData[TANKPRE], this->kneSupPre))
        {
            this->com->comArray[PIDACTTEST] = false;
        }
    }
    else if (joint == 3)
    {
        this->ValveOn(this->RBalVal);
        if (this->CheckSupPre_main(this->RAnkPreVal, this->senData[RANKPRE], this->senData[TANKPRE], this->ankSupPre))
        {
            this->com->comArray[PIDACTTEST] = false;
        }
    }
    else
    {
        std::cout << "wrong command value\n";
    }
}
void Controller::PIDRecTest(int desPre, int joint)
{
    if (joint == 0)
    {
    }
    else if (joint == 1)
    {
    }
    else if (joint == 2)
    {
    }
    else if (joint == 3)
    {
    }
    else
    {
        std::cout << "wrong command value\n";
    }
}

// Data pre-process for the PID controller (feedback linearization?)
//===========================================================================================
float Controller::SupPreInput(int knePre, int tankPre, int desPre)
{

    return (float)(desPre - knePre) / (tankPre - knePre);
}
float Controller::AnkActInput(int ankPre, int tankPre)
{
    return (float)(this->ankActPre - ankPre) / (tankPre - ankPre);
}
float Controller::AnkPreRecInput(int ankPre, int tankPre)
{
    return (float)ankPre / tankPre;
}
float Controller::KnePreRecInput(int knePre, int tankPre)
{
    return (float)knePre / tankPre;
}

// Finite state machine
//===================================================================================================================

void Controller::BipedEngRec()
{

    // int curHipDiff = this->senData[LHIPPOS] + this->senData[RHIPPOS] - this->LHipMean - this->RHipMean;

    // if (this->gaitStart)
    // {
    //     bool curGaitEnd =false;

    //     {
    //         std::lock_guard<std::mutex> lock(*this->gaitEndLock);
    //         curGaitEnd = this->gaitEnd;
    //     }

    //     this->FSM->PushSen(this->senData);

    //     if ((this->FSM->IsReady() && curGaitEnd))
    //     {
    //         if(this->singleGaitJoin){
    //             // this->SingleGait_th->join();
    //             this->singleGaitJoin = false;
    //             //this->SingleGait_th.reset();
    //         }
    //         std::cout << "start thread\n";
    //         {
    //             std::lock_guard<std::mutex> lock(*this->gaitEndLock);
    //             this->gaitEnd = false;
    //         }

    //         this->SingleGait_th.reset(new std::thread(&Controller::SingleGaitPeriod, this));
    //         this->SingleGait_th->detach();
    //         this->singleGaitJoin = true;
    //     }
    //     else{
    //         //std::cout << "not ready\n";
    //     }
    // }
    // else
    // {
    //     this->gaitStart = true; //this is for preHipDiff have initial value
    // }

    // this->preHipDiff = curHipDiff;

    //test, assume the function is trigger every time is is done
    if(!this->sw_FSM){
        this->FSM_start();
    }
    
}

// 7 Phases of single leg's walking gait
void Controller::Init_swing(char side)
{
    if (side == 'r')
    {
        this->ValveOff(this->RKneVal);
        
        this->SetDuty(this->RAnkPreVal, 0);
        {
            std::lock_guard<std::mutex> curLock(this->com->comLock);
            this->com->comArray[CON_RANK_ACT] = false;
        }
    }
    else
    {
        this->ValveOff(this->LKneVal);
        this->SetDuty(this->LAnkPreVal, 0);
        {
            std::lock_guard<std::mutex> curLock(this->com->comLock);
            this->com->comArray[CON_LANK_ACT] = false;
        }
    }
}
void Controller::Mid_swing(char side)
{
    if (side == 'r')
    {
        this->ValveOff(this->RBalVal);
        this->ValveOn(this->RKneVal);
    }
    else
    {
        this->ValveOff(this->LBalVal);
        this->ValveOn(this->LKneVal);
    }
}
void Controller::Term_swing(char side)
{
    if (side == 'r')
    {
        this->ValveOn(this->RBalVal);
        {
            std::lock_guard<std::mutex> curLock(this->com->comLock);
            this->com->comArray[CON_RKNE_SUP] = true;
        }
        
    }
    else
    {
        this->ValveOn(this->LKneVal);
        {
            std::lock_guard<std::mutex> curLock(this->com->comLock);
            this->com->comArray[CON_LKNE_SUP] = true;
        }
        
        // this->CheckSupPre(this->LKnePreVal, this->senData[LKNEPRE], this->senData[TANKPRE]);
    }
}
void Controller::Load_resp(char side)
{
    if (side == 'r')
    {
        std::lock_guard<std::mutex> curLock(this->com->comLock);
        this->com->comArray[CON_RKNE_SUP] = false;

        this->com->comArray[CON_RKNE_REC] = true;
        this->com->comArray[CON_RANK_REC] = true;
    }
    else
    {
        std::lock_guard<std::mutex> curLock(this->com->comLock);
        this->com->comArray[CON_LKNE_SUP] = false;
        this->com->comArray[CON_LKNE_REC] = true;
        this->com->comArray[CON_LANK_REC] = true;
        // this->KnePreRec(this->LKnePreVal, this->senData[LKNEPRE], this->senData[TANKPRE]);
        // this->AnkPreRec(this->LAnkPreVal,this->senData[LANKPRE],this->senData[TANKPRE],this->LBalVal);
    }
}
void Controller::Mid_stance(char side)
{
    // make sure the pressure valves stops
    if (side == 'r')
    {
        //this is needed since the previous stage is load response
        this->SetDuty(this->RKnePreVal, 0);
        this->SetDuty(this->RAnkPreVal, 0);
        {
            std::lock_guard<std::mutex> curLock(this->com->comLock);
            this->com->comArray[CON_RKNE_REC] = false;
            this->com->comArray[CON_RANK_REC] = false;
        }
        
        this->ValveOff(this->RBalVal);
    }
    else
    {
        this->SetDuty(this->LKnePreVal, 0);
        this->SetDuty(this->LAnkPreVal, 0);
        {
            std::lock_guard<std::mutex> curLock(this->com->comLock);
            this->com->comArray[CON_LKNE_REC] = false;
            this->com->comArray[CON_LANK_REC] = false;
        }
        
        this->ValveOff(this->LBalVal);
    }
}
void Controller::Term_stance(char side)
{
    if (side == 'r')
    {
        this->ValveOn(this->RBalVal);
    }
    else
    {
        this->ValveOn(this->LBalVal);
    }
}
void Controller::Pre_swing(char side)
{
    if (side == 'r')
    {
        std::lock_guard<std::mutex> curLock(this->com->comLock);
        this->com->comArray[CON_RANK_ACT] = true;
    }
    else
    {
        std::lock_guard<std::mutex> curLock(this->com->comLock);
        this->com->comArray[CON_LANK_ACT] = true;
    }
}

// Actions
// detail of how pressure valve will operate is defined here
//===========================================================================================

void Controller::KnePreRec_main(std::shared_ptr<PWMGen> knePreVal, int knePre, int tankPre, int desPre)
{
    if ((knePre - tankPre > 10) && (knePre > desPre))
    {
        if (!this->kneRecPID_set)
        {
            this->kneRecPID_set = true;
            knePreVal->SetPID_const(100, 0.01, 0.001,this->KnePreRecInput(knePre, tankPre));
        }
        else{
            knePreVal->PushMea(this->senData[TIME],this->KnePreRecInput(knePre, tankPre));
            this->pwmDuty[knePreVal->GetIdx()] = knePreVal->duty.byte[0];
        }
    }
    else
    {
        this->kneRecPID_set = false;
        knePreVal->PushMea(this->senData[TIME], -500.0f);
        this->pwmDuty[knePreVal->GetIdx()] = knePreVal->duty.byte[0];
        
    }
}

void Controller::AnkPreRec_main(std::shared_ptr<PWMGen> ankPreVal, int ankPre, int tankPre, std::shared_ptr<Valve> balVal, int desPre)
{
    //When recycle ankle pressure, if the pressure is too high, we direct the ankle pressure to the knee joint
    if ((ankPre - tankPre > 10) && (ankPre > desPre))
    {
        if (!this->ankRecPID_set)
        {
            this->ankRecPID_set = true;
            ankPreVal->SetPID_const(100, 0.01, 0.001, this->AnkPreRecInput(ankPre, tankPre));
            // this->ankRecPID.reset(new PIDCon(100, 0.1, 0.001, this->AnkPreRecInput(ankPre, tankPre)));
            this->ValveOn(balVal);
        }
        else{
            ankPreVal->PushMea(this->senData[TIME],this->AnkPreRecInput(ankPre, tankPre));
            this->pwmDuty[ankPreVal->GetIdx()] = ankPreVal->duty.byte[0];
        }
    }
    else
    {
        this->ankRecPID_set = false;
        ankPreVal->PushMea(this->senData[TIME], -500.0f);
        this->pwmDuty[ankPreVal->GetIdx()] = ankPreVal->duty.byte[0];
    }
}

bool Controller::CheckSupPre_main(std::shared_ptr<PWMGen> preVal, int knePre, int tankPre, int desPre)
{
    if (knePre - desPre < 10)
    {
        if (!this->kneSupPID_set)
        {
            this->kneSupPID_set = true;
            preVal->SetPID_const(100, 0.1, 0.01, this->SupPreInput(knePre, tankPre, desPre));
        }
        else
        {
            preVal->PushMea(this->senData[TIME], this->SupPreInput(knePre, tankPre, desPre));
            this->pwmDuty[preVal->GetIdx()] = preVal->duty.byte[0];
        }
        return false;
    }
    else
    {
        preVal->PushMea(this->senData[TIME], -500.0f);
        this->pwmDuty[preVal->GetIdx()] = preVal->duty.byte[0];
        this->kneSupPID_set = false;
        return true;
    }
}
void Controller::AnkPushOff_main(std::shared_ptr<PWMGen> ankPreVal, int ankPre, int tankPre)
{
    //std::cout << "running valves" << ankPreVal->GetIdx() << std::endl;
    if (this->ankActPre - ankPre < 10)
    {
        if (!this->ankActPID_set)
        {
            ankActPID_set = true;
            ankPreVal->SetPID_const(800, 0.01, 0.001, this->AnkActInput(ankPre, tankPre));
            // this->ankActPID.reset(new PIDCon(800, 0.01, 0.001, this->AnkActInput(ankPre, tankPre)));
        }
        else
        {
            ankPreVal->PushMea(this->senData[TIME], this->AnkActInput(ankPre, tankPre));
            this->pwmDuty[ankPreVal->GetIdx()] = ankPreVal->duty.byte[0];
        }
    }
    else
    {
        ankPreVal->PushMea(this->senData[TIME], -500.0f);
        this->pwmDuty[ankPreVal->GetIdx()] = ankPreVal->duty.byte[0];
    }
}
//Time based FSM
void Controller::SingleGaitPeriod()
{
    std::cout << "start gait\n";
    clock_gettime(CLOCK_MONOTONIC, &this->gaitTimer);
    this->FSM->GetPhaseTime(this->senData[TIME], this->p1_t, this->p2_t, this->p3_t, this->p4_t, this->p5_t, this->p6_t, this->p7_t, this->p8_t, this->p9_t, this->p10_t);
    //std::cout << "total wait time= " << (long long)this->p1_t + this->p2_t + this->p3_t + this->p4_t + this->p5_t + this->p6_t + this->p7_t + this->p8_t + this->p9_t + this->p10_t << "ns" << std::endl;
    // this->gaitTimer.tv_nsec += (10*this->p1_t);
    // Common::tsnorm(&this->gaitTimer);
    // clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &this->gaitTimer, NULL);

    //phase 5
    this->gaitTimer.tv_nsec += this->p5_t;

    Common::tsnorm(&this->gaitTimer);
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &this->gaitTimer, NULL);
    // this->Term_stance('l');
    // this->Term_swing('r');
    std::cout << "pass phase 5\n";
    //phase 6
    this->gaitTimer.tv_nsec += this->p6_t;
    Common::tsnorm(&this->gaitTimer);
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &this->gaitTimer, NULL);
    // this->Load_resp('r');
    std::cout << "pass phase 6\n";
    //phase 7
    this->gaitTimer.tv_nsec += this->p7_t;

    Common::tsnorm(&this->gaitTimer);
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &this->gaitTimer, NULL);
    // this->Pre_swing('l');
    std::cout << "pass phase 7\n";
    //phase 8
    this->gaitTimer.tv_nsec += this->p8_t;

    Common::tsnorm(&this->gaitTimer);
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &this->gaitTimer, NULL);
    
    // this->Init_swing('l');
    std::cout << "pass phase 8\n";
    //phase 9
    this->gaitTimer.tv_nsec += this->p9_t;

    Common::tsnorm(&this->gaitTimer);
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &this->gaitTimer, NULL);
    // this->Mid_swing('l');
    std::cout << "pass phase 9\n";
    //phase 10
    this->gaitTimer.tv_nsec += this->p10_t;

    Common::tsnorm(&this->gaitTimer);
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &this->gaitTimer, NULL);
    
    // this->Term_swing('l');
    // this->Term_stance('r');
    std::cout << "pass phase 10\n";
    //phase 1

    this->gaitTimer.tv_nsec += this->p1_t;
    Common::tsnorm(&this->gaitTimer);
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &this->gaitTimer, NULL);
    
    // this->Load_resp('l');
    std::cout << "pass phase 1\n";
    //phase 2
    this->gaitTimer.tv_nsec += this->p2_t;
    Common::tsnorm(&this->gaitTimer);
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &this->gaitTimer, NULL);
    
    // this->Mid_stance('l');
    // this->Pre_swing('r');
    std::cout << "pass phase 2\n";
    //phase 3
    this->gaitTimer.tv_nsec += this->p3_t;
    Common::tsnorm(&this->gaitTimer);
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &this->gaitTimer, NULL);
    
    // this->Init_swing('r');
    std::cout << "pass phase 3\n";
    //phase 4
    this->gaitTimer.tv_nsec += this->p4_t;
    Common::tsnorm(&this->gaitTimer);
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &this->gaitTimer, NULL);

    // this->Mid_swing('r');
    std::cout << "gait ends\n";
    {
        std::lock_guard<std::mutex> curLock(this->gaitEndLock);
        this->gaitEnd = true;
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
        this->conMain_th = std::thread(&Controller::ConMainLoop, this, senData, senRaw,senDataLock);
        this->sw_conMain = true;
    }
    else
    {
        std::cout << "controller already started\n";
    }
}
void Controller::Stop()
{
    {
        std::lock_guard<std::mutex> curLock(this->conSW_lock);
        this->sw_conMain = false;
    }
    this->conMain_th.join();
}

void Controller::FSMLoop(){
    while(true){

        
        bool curCond;
        {
            std::lock_guard<std::mutex> curLock(this->FSMLock);
            curCond = this->sw_FSM;
        }
        if(!curCond){
            break;
        }
        
        bool curGaitEnd;
        {
            std::lock_guard<std::mutex> curLock(this->gaitEndLock);
            curGaitEnd = this->gaitEnd;
        }
        if(curGaitEnd){
            this->gaitEnd = false;
            this->SingleGaitPeriod();
        }
    }
}
void Controller::FSM_start(){
    bool curCond;
    {
        std::lock_guard<std::mutex> curLock(this->FSMLock);
        curCond = this->sw_FSM;
    }
    if(!curCond){
        this->sw_FSM = true;
        this->FSMLoop_th.reset(new std::thread(&Controller::FSMLoop, this));
    }
    
}
void Controller::FSM_stop(){
    {
        std::lock_guard<std::mutex> curLock(this->FSMLock);
        this->sw_FSM = false;
    }
    if(this->FSMLoop_th){
        if(this->FSMLoop_th->joinable()){
             this->FSMLoop_th->join();
        }
    }      
}