#include "Controller.h"
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
    float curMea = (float)(ankPre - this->ankRecPre) / (ankPre - tankPre);
    return curMea;
}
float Controller::KnePreRecInput(int knePre, int tankPre)
{
    float curMea = (float)(knePre - this->kneRecPre) / (knePre - tankPre);
    
    return curMea;
}
