#ifndef PIDCON_H
#define PIDCON_H
class PIDCon
{
private:
    float kp;
    float kd;
    float ki;
    float preMea; //we only need 1-D data for memory since we only need 1 output, PID controller should also have 1-D input
    unsigned long preTime;
    float sumErr = 0;
public:
    PIDCon(float _kp,float _kd,float _ki,float firstMea);
    ~PIDCon();
    int GetDuty(float curMea);
    void SetInit(float curMea);
    void UpDatePID(float _kp,float _ki, float _kd);
};


#endif