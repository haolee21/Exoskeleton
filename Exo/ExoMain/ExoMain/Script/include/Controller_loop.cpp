#include "Controller.h"
// main loop
// =================================================================================================================
void Controller::ConMainLoop(int *_senData, char *senRaw, std::mutex *senDataLock)
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
            if (this->com->comArray[TESTVAL]){
                this->TestValve();
                //taskQue.push(std::thread(&Controller::TestValve, this));
            }
                
                
            if (this->com->comArray[TESTPWM])
                this->TestPWM();
                // taskQue.push(std::thread(&Controller::TestPWM, this));
            if (this->com->comArray[SHUTPWM])
            {
                // taskQue.push(std::thread(&Controller::ShutDownPWM, this));
                this->ShutDownPWM();
                this->com->comArray[SHUTPWM] = false;
            }

            if (this->com->comArray[KNEMODSAMP])
            {
                // taskQue.push(std::thread(&Controller::SampKneMod, this, com->comVal[KNEMODSAMP]));
                this->SampKneMod(com->comVal[KNEMODSAMP]);
            }
            if (this->com->comArray[KNEPREREL])
            {
                // taskQue.push(std::thread(&Controller::KneRel, this));
                this->KneRel();
            }
            if (this->com->comArray[TESTALLLEAK])
            {
                // taskQue.push(std::thread(&Controller::TestAllLeak, this));
                this->TestAllLeak();
                this->com->comArray[TESTALLLEAK] = false;
            }
            if (this->com->comArray[FREEWALK])
            {
                // taskQue.push(std::thread(&Controller::FreeWalk, this));
                this->FreeWalk();
                this->com->comArray[FREEWALK] = false;
            }
            if (this->com->comArray[TESTLEAK])
            {
                // taskQue.push(std::thread(&Controller::TestLeak, this, com->comVal[TESTLEAK]));
                this->TestLeak(com->comVal[TESTLEAK]);
                this->com->comArray[TESTLEAK] = false;
            }
            if (this->com->comArray[TESTLANK])
            {
                // taskQue.push(std::thread(&Controller::TestLAnk, this));
                this->TestLAnk();
                this->com->comArray[TESTLANK] = false;
            }
            if (this->com->comArray[TESTRANK])
            {
                // taskQue.push(std::thread(&Controller::TestRAnk, this));
                this->TestRAnk();
                this->com->comArray[TESTRANK] = false;
            }
            if (this->com->comArray[SHOWSEN])
            {
                // taskQue.push(std::thread(&Controller::ShowSen, this));
                this->ShowSen();
                this->com->comArray[SHOWSEN] = false;
            }
            if (this->com->comArray[BIPEDREC])
            {
                // taskQue.push(std::thread(&Controller::BipedEngRec, this));
                this->BipedEngRec();
            }
            if (this->com->comArray[TESTSYNC])
            {
                // taskQue.push(std::thread(&Controller::TestReactingTime, this));
                this->TestReactingTime();
            }
            if (this->com->comArray[PIDACTTEST])
            {
                // taskQue.push(std::thread(&Controller::PIDActTest, this, this->com->comVal[PIDACTTEST]));
                this->PIDActTest(this->com->comVal[PIDACTTEST]);
            }
            if (this->com->comArray[PIDRECTEST])
            {
                taskQue.push(std::thread(&Controller::PIDRecTest, this, this->com->comVal[PIDRECTEST]));
                this->PIDRecTest(this->com->comVal[PIDRECTEST]);
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
                    // taskQue.push(std::thread(&Controller::TestOnePWM, this, this->com->comVal[TESTONEPWM]));
                    this->TestOnePWM(this->com->comVal[TESTONEPWM]);
                    this->testOnePWM_flag = true;
                }
                this->com->comArray[TESTONEPWM] = false;
            }
            if (this->com->comArray[CON_LANK_ACT])
            {
                // taskQue.push(std::thread(&Controller::AnkPushOff_main, this, this->LAnkPreVal, this->senData[LANKPRE], this->senData[TANKPRE]));
                this->AnkPushOff_main(this->LAnkPreVal,this->senData[LANKPRE],this->senData[TANKPRE]);
            }
            if (this->com->comArray[CON_RANK_ACT])
            {
                // taskQue.push(std::thread(&Controller::AnkPushOff_main, this, this->RAnkPreVal, this->senData[RANKPRE], this->senData[TANKPRE]));
                this->AnkPushOff_main(this->RAnkPreVal,this->senData[RANKPRE],this->senData[TANKPRE]);
            }
            if (this->com->comArray[CON_LKNE_REC])
            {
                // taskQue.push(std::thread(&Controller::CheckKnePreRec_main, this, this->LKnePreVal, this->senData[LKNEPRE], this->senData[TANKPRE], this->kneRecPre));
                this->CheckKnePreRec_main(this->LKnePreVal,this->senData[LKNEPRE],this->senData[TANKPRE],this->kneRecPre);
            }
            if (this->com->comArray[CON_RKNE_REC])
            {
                // taskQue.push(std::thread(&Controller::CheckKnePreRec_main, this, this->RKnePreVal, this->senData[RKNEPRE], this->senData[TANKPRE], this->kneRecPre));
                this->CheckKnePreRec_main(this->RKnePreVal,this->senData[RKNEPRE],this->senData[TANKPRE],this->kneRecPre);
            }
            if (this->com->comArray[CON_LANK_REC])
            {
                // taskQue.push(std::thread(&Controller::
                this->CheckAnkPreRec_main(this->LAnkPreVal, this->senData[LANKPRE], this->senData[TANKPRE], this->LBalVal, this->ankRecPre);
            }
            if (this->com->comArray[CON_RANK_REC])
            {
                // taskQue.push(std::thread(&Controller::
                this->CheckAnkPreRec_main(this->RAnkPreVal, this->senData[RANKPRE], this->senData[TANKPRE], this->RBalVal, this->ankRecPre);
            }
            if (this->com->comArray[CON_LKNE_SUP])
            {
                // taskQue.push(std::thread(&Controller::
                this->CheckSupPre_main(this->LKnePreVal, this->senData[LKNEPRE], this->senData[TANKPRE], this->kneSupPre);
            }
            if (this->com->comArray[CON_RKNE_SUP])
            {
                // taskQue.push(std::thread(&Controller::
                this->CheckSupPre_main(this->RKnePreVal, this->senData[RKNEPRE], this->senData[TANKPRE], this->kneSupPre);
            }
            if (this->com->comArray[CON_LANK_SUP])
            {
                // taskQue.push(std::thread(&Controller::
                this->CheckSupPre_main(this->LAnkPreVal, this->senData[LANKPRE], this->senData[TANKPRE], this->ankSupPre);
            }
            if (this->com->comArray[CON_RANK_SUP])
            {
                // taskQue.push(std::thread(&Controller::
                this->CheckSupPre_main(this->RAnkPreVal, this->senData[RANKPRE], this->senData[TANKPRE], this->ankSupPre);
            }
            if (this->com->comArray[CON_SET_INIT_POS])
            {
                this->com->comArray[CON_SET_INIT_POS] = false;
                // taskQue.push(std::thread(&FSMachine::SetInitPos, this->FSM.get(), this->senData[LANKPOS], this->senData[RANKPOS]));
                this->FSM->SetInitPos(this->senData[LHIPPOS],this->senData[RHIPPOS]);
            }
        }
        if (this->display)
        {
            
            // taskQue.push(std::thread(&Controller::
            this->SendToDisp(senRaw);
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
        if (!cond)
        {
            break;
        }

        conTimer.tv_nsec += this->sampT;
        Common::tsnorm(&conTimer);
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &conTimer, NULL);
    } 
}