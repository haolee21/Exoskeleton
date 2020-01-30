#include "Controller.h"
void Controller::FSMLoop()
{
    struct timespec t_FSM;
    clock_gettime(CLOCK_MONOTONIC, &t_FSM);
   

    while (true)
    {

        bool curCond;
        {
            std::lock_guard<std::mutex> curLock(this->FSMLock);
            curCond = this->sw_FSM;
        }
        if (!curCond)
        {
            std::cout << "FSM should stop\n";
            break;
        }

        //check if we need to start new gait

        bool needNewGait;
        {
            std::lock_guard<std::mutex> curLock(this->gaitStartLock);
            needNewGait = this->startNewGait;
        }
        if (needNewGait)
        {

            {
                std::lock_guard<std::mutex> curLock(this->gaitStartLock);
                this->startNewGait = false;
            }

            //gait_th = std::thread(&Controller::SingleGaitPeriod, this);
            this->SingleGaitPeriod(); //I don't understand why I originally need a seperate thread
            // needJoin = true;
            clock_gettime(CLOCK_MONOTONIC, &t_FSM); //re initialize the timer since it must passes many sampT after a gait ends
        }

        t_FSM.tv_nsec += this->sampT; //we need a waittime here, otherwise the program will iterate in max speed and result in deadlock
        Common::tsnorm(&t_FSM);
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t_FSM, NULL);
        //std::cout << "fsm cond: " << curCond << std::endl;
    }

    std::cout << "end FSM Loop\n";
}
void Controller::FSM_start()
{
    bool curCond;
    {
        std::lock_guard<std::mutex> curLock(this->FSMLock);
        curCond = this->sw_FSM;
    }
    if (!curCond)
    {
        this->sw_FSM = true;
        this->FSMLoop_th.reset(new std::thread(&Controller::FSMLoop, this));
        this->FSM_flag = true;
    }
}
void Controller::FSM_stop()
{
    if (this->FSM_flag)
    {
        bool curFSM_sw;
        {
            std::lock_guard<std::mutex> curLock(this->FSMLock);
            curFSM_sw = this->sw_FSM;
        }
        if (curFSM_sw)
        {
            {
                std::lock_guard<std::mutex> curLock(this->FSMLock);
                this->sw_FSM = false;
            }

            if (this->FSMLoop_th->joinable())
            {
                this->FSMLoop_th->join();
            }
        }
        std::cout << "FSM loop stops\n";
    }
    else
    {
        std::cout << "FSM was not activated\n";
    }
}
// Finite state machine
//===================================================================================================================

void Controller::BipedEngRec()
{

    int curHipDiff = this->mvf.DataFilt(this->senData[LHIPPOS] + this->senData[RHIPPOS] - this->LHipMean - this->RHipMean);
    //std::cout << "hipdiff: " << curHipDiff << std::endl;
    if ((curHipDiff < 0) && (this->preHipDiff > 0))
    {

        bool needStartNewGait;
        {
            std::scoped_lock<std::mutex> curLock(this->gaitStartLock);
            //std::lock_guard<std::mutex> curLock(this->gaitStartLock);
            needStartNewGait = this->startNewGait;
        }
        if (!needStartNewGait)
        {
            std::vector<bool> swTemp;
            swTemp.push_back(true);
            this->FSM->GetPhaseTime(this->senData[TIME], this->p1_t, this->p2_t, this->p3_t, this->p4_t, this->p5_t, this->p6_t, this->p7_t, this->p8_t, this->p9_t, this->p10_t);
            this->swGaitRec->PushData((unsigned long)this->senData[TIME], swTemp);
            {
                std::scoped_lock<std::mutex> curLock(this->gaitStartLock);
                this->startNewGait = true;
                
            }
            std::cout << "new step\n";
        }
    }
    else
    {
        //std::cout << "push data to fsm\n";
        this->FSM->PushSen(this->senData);
    }

    this->preHipDiff = curHipDiff;

    //test, assume the function is trigger every time is is done
    if (!this->sw_FSM)
    {
        this->FSM_start();
    }
}

// 7 Phases of single leg's walking gait
void Controller::Init_swing(char side)
{
    if (side == 'r')
    {
        this->ValveOff(this->RKneVal);

        // this->SetDuty(this->RAnkPreVal, 0);
        {
            std::scoped_lock<std::mutex> curLock(this->com->comLock);
            this->com->comArray[CON_RANK_ACT] = false;
        }
        this->RAnkPreVal->PushMea(this->senData[TIME], -500.0f);
        this->pwmDuty[this->RAnkPreVal->GetIdx()] = this->RAnkPreVal->duty.byte[0];
    }
    else
    {
        this->ValveOff(this->LKneVal);
        // this->SetDuty(this->LAnkPreVal, 0);
        {
            std::scoped_lock<std::mutex> curLock(this->com->comLock);
            this->com->comArray[CON_LANK_ACT] = false;
        }
        this->LAnkPreVal->PushMea(this->senData[TIME], -500.0f);
        this->pwmDuty[this->LAnkPreVal->GetIdx()] = this->LAnkPreVal->duty.byte[0];
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
        this->ValveOn(this->LBalVal);
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
        //this->com->comArray[CON_RANK_REC] = true;
    }
    else
    {
        std::lock_guard<std::mutex> curLock(this->com->comLock);
        this->com->comArray[CON_LKNE_SUP] = false;
        this->com->comArray[CON_LKNE_REC] = true;
        //this->com->comArray[CON_LANK_REC] = true;
        // this->KnePreRec(this->LKnePreVal, this->senData[LKNEPRE], this->senData[TANKPRE]);
        // this->AnkPreRec(this->LAnkPreVal,this->senData[LANKPRE],this->senData[TANKPRE],this->LBalVal);
    }
}
void Controller::Mid_stance(char side)
{
    this->ankRecPID_set = false;
    this->kneRecPID_set = false;
    // make sure the pressure valves stops
    if (side == 'r')
    {
        //this is needed since the previous stage is load response
        {
            std::lock_guard<std::mutex> curLock(this->com->comLock);
            this->com->comArray[CON_RKNE_REC] = false;
            this->com->comArray[CON_RANK_REC] = false;
        }
        this->RKnePreVal->PushMea(this->senData[TIME], -500.0f);
        this->RAnkPreVal->PushMea(this->senData[TIME], -500.0f);
        this->pwmDuty[RKnePreVal->GetIdx()] = RKnePreVal->duty.byte[0];
        this->pwmDuty[RAnkPreVal->GetIdx()] = RAnkPreVal->duty.byte[0];
        this->ValveOff(this->RBalVal);
    }
    else
    {
        {
            std::lock_guard<std::mutex> curLock(this->com->comLock);
            this->com->comArray[CON_LKNE_REC] = false;
            this->com->comArray[CON_LANK_REC] = false;
        }
        this->LKnePreVal->PushMea(this->senData[TIME], -500.0f);
        this->LAnkPreVal->PushMea(this->senData[TIME], -500.0f);
        this->pwmDuty[LKnePreVal->GetIdx()] = LKnePreVal->duty.byte[0];
        this->pwmDuty[LAnkPreVal->GetIdx()] = LAnkPreVal->duty.byte[0];
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
//Time based FSM
void Controller::SingleGaitPeriod()
{

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
    this->Term_stance('l');

    this->Term_swing('r'); //suspect1 pass

    //phase 6
    this->gaitTimer.tv_nsec += this->p6_t;
    Common::tsnorm(&this->gaitTimer);
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &this->gaitTimer, NULL);
    this->Load_resp('r'); //suspect2 pass

    //phase 7
    this->gaitTimer.tv_nsec += this->p7_t;

    Common::tsnorm(&this->gaitTimer);
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &this->gaitTimer, NULL);
    this->Pre_swing('l');
    this->Mid_stance('r');
    //phase 8
    this->gaitTimer.tv_nsec += this->p8_t;

    Common::tsnorm(&this->gaitTimer);
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &this->gaitTimer, NULL);

    this->Init_swing('l');

    //phase 9
    this->gaitTimer.tv_nsec += this->p9_t;

    Common::tsnorm(&this->gaitTimer);
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &this->gaitTimer, NULL);
    this->Mid_swing('l');

    //phase 10
    this->gaitTimer.tv_nsec += this->p10_t;

    Common::tsnorm(&this->gaitTimer);
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &this->gaitTimer, NULL);

    this->Term_swing('l');
    this->Term_stance('r');

    //phase 1

    this->gaitTimer.tv_nsec += this->p1_t;
    Common::tsnorm(&this->gaitTimer);
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &this->gaitTimer, NULL);

    this->Load_resp('l');

    //phase 2
    this->gaitTimer.tv_nsec += this->p2_t;
    Common::tsnorm(&this->gaitTimer);
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &this->gaitTimer, NULL);

    this->Mid_stance('l');
    this->Pre_swing('r');

    //phase 3
    this->gaitTimer.tv_nsec += this->p3_t;
    Common::tsnorm(&this->gaitTimer);
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &this->gaitTimer, NULL);

    this->Init_swing('r');

    //phase 4
    this->gaitTimer.tv_nsec += this->p4_t;
    Common::tsnorm(&this->gaitTimer);
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &this->gaitTimer, NULL);

    this->Mid_swing('r');

    // {
    //     std::lock_guard<std::mutex> curLock(this->gaitEndLock);
    //     this->gaitEnd = true;
    // }
}
