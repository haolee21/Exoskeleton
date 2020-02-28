#include "Controller.h"
// Actions
// detail of how pressure valve will operate is defined here
//===========================================================================================

bool Controller::CheckKnePreRec_main(std::shared_ptr<PWMGen> knePreVal, int knePre, int tankPre, int desPre)
{

    if ((knePre - tankPre > 10) && (knePre > desPre))
    {
        float curMea = this->KnePreRecInput(knePre, tankPre);
        if (!this->kneRecPID_set)
        {
            this->kneRecPID_set = true;
            knePreVal->SetPID_const(150, 0.1, 0.001, curMea);
        }
        else
        {
            knePreVal->PushMea(this->senData[TIME], curMea);
            this->pwmDuty[knePreVal->GetIdx()] = knePreVal->duty.byte[0];
        }
        return false;
    }
    else
    {
        this->kneRecPID_set = false;
        knePreVal->PushMea(this->senData[TIME], -500.0f);
        this->pwmDuty[knePreVal->GetIdx()] = knePreVal->duty.byte[0];
        return true;
    }
}

bool Controller::CheckAnkPreRec_main(std::shared_ptr<PWMGen> ankPreVal, int ankPre, int tankPre, std::shared_ptr<Valve> balVal, int desPre)
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
        else
        {
            ankPreVal->PushMea(this->senData[TIME], this->AnkPreRecInput(ankPre, tankPre));
            this->pwmDuty[ankPreVal->GetIdx()] = ankPreVal->duty.byte[0];
        }
        return false;
    }
    else
    {
        this->ankRecPID_set = false;
        ankPreVal->PushMea(this->senData[TIME], -500.0f);
        this->pwmDuty[ankPreVal->GetIdx()] = ankPreVal->duty.byte[0];
        return true;
    }
}

bool Controller::CheckSupPre_main(std::shared_ptr<PWMGen> preVal, int knePre, int tankPre, int desPre)
{
    if (knePre - desPre < 10)
    {
        if (!this->kneSupPID_set)
        {
            this->kneSupPID_set = true;
            preVal->SetPID_const(100, 0.5, 0.01, this->SupPreInput(knePre, tankPre, desPre));
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