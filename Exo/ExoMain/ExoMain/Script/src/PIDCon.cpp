#include "PIDCon.h"
PIDCon::PIDCon(float _kp,float _kd,float _ki,float firstMea)
{
    this->kp = _kp;
    this->kd = _kd;
    this->ki = _ki;
    this->SetInit(firstMea);
}
void PIDCon::UpDatePID(float _kp,float _kd,float _ki){
    this->kp = _kp;
    this->ki = _ki;
    this->kd = _kd;
}
PIDCon::~PIDCon()
{
}
int PIDCon::GetDuty(float curMea){
    int duty;
    duty = this->kp*curMea + this->kd*(curMea - this->preMea)+this->ki*this->sumErr;

    this->preMea = curMea;
    this->sumErr+=curMea;
    if(duty>100){
        duty = 100;
    }
    else if(duty <0){
        duty =0;
    }
    return duty;
  
}
void PIDCon::SetInit(float curMea){
    this->preMea = curMea;
}