#include "Controller.h"


// Finite state machine
//===================================================================================================================

void Controller::BipedEngRec()
{
    this->FSM->PushSen(this->senData);
    
    // int curHipDiff = this->mvf.DataFilt(this->senData[LANKPOS] + this->senData[RANKPOS] - this->LHipMean - this->RHipMean);
    // //std::cout << "hipdiff: " << curHipDiff << std::endl;
    // if ((curHipDiff < 0) && (this->preHipDiff > 0))
    // {

    //     bool needStartNewGait;
    //     {
    //         std::scoped_lock<std::mutex> curLock(this->gaitStartLock);
    //         //std::lock_guard<std::mutex> curLock(this->gaitStartLock);
    //         needStartNewGait = this->startNewGait;
    //     }
    //     if (!needStartNewGait)
    //     {
    //         std::vector<bool> swTemp;
    //         swTemp.push_back(true);
    //         this->FSM->GetPhaseTime(this->senData[TIME], this->p1_t, this->p2_t, this->p3_t, this->p4_t, this->p5_t, this->p6_t, this->p7_t, this->p8_t, this->p9_t, this->p10_t);
    //         this->swGaitRec->PushData((unsigned long)this->senData[TIME], swTemp);
    //         {
    //             std::scoped_lock<std::mutex> curLock(this->gaitStartLock);
    //             this->startNewGait = true;
                
    //         }
    //         std::cout << "new step\n";
    //     }
    // }
    // else
    // {
    //     //std::cout << "push data to fsm\n";
        
    // }

    // this->preHipDiff = curHipDiff;


}

// 7 Phases of single leg's walking gait
void Controller::Init_swing(char side)
{
    if (side == 'r')
    {
        this->ValveOff(this->RKneVal);

        // this->SetDuty(this->RAnkPreVal, 0);
        this->com->comArray[CON_RANK_ACT] = false;
        
        this->RAnkPreVal->PushMea(this->senData[TIME], -500.0f);
        this->pwmDuty[this->RAnkPreVal->GetIdx()] = this->RAnkPreVal->duty.byte[0];
    }
    else
    {
        this->ValveOff(this->LKneVal);
        // this->SetDuty(this->LAnkPreVal, 0);
        this->com->comArray[CON_LANK_ACT] = false;
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
        this->com->comArray[CON_RKNE_SUP] = true;
        
    }
    else
    {
        this->ValveOn(this->LBalVal);  
        this->com->comArray[CON_LKNE_SUP] = true;
        

        // this->CheckSupPre(this->LKnePreVal, this->senData[LKNEPRE], this->senData[TANKPRE]);
    }
}
void Controller::Load_resp(char side)
{
    if (side == 'r')
    {
        
        this->com->comArray[CON_RKNE_SUP] = false;

        this->com->comArray[CON_RKNE_REC] = true;
        //this->com->comArray[CON_RANK_REC] = true;
    }
    else
    {
        
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
        
            
        this->com->comArray[CON_RKNE_REC] = false;
        this->com->comArray[CON_RANK_REC] = false;
    
        this->RKnePreVal->PushMea(this->senData[TIME], -500.0f);
        this->RAnkPreVal->PushMea(this->senData[TIME], -500.0f);
        this->pwmDuty[RKnePreVal->GetIdx()] = RKnePreVal->duty.byte[0];
        this->pwmDuty[RAnkPreVal->GetIdx()] = RAnkPreVal->duty.byte[0];
        this->ValveOff(this->RBalVal);
    }
    else
    {
        
            
        this->com->comArray[CON_LKNE_REC] = false;
        this->com->comArray[CON_LANK_REC] = false;
        
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
        
        this->com->comArray[CON_RANK_ACT] = true;
    }
    else
    {
        
        this->com->comArray[CON_LANK_ACT] = true;
    }
}
//Time based FSM

