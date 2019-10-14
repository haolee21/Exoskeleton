#include "Valve.h"

void Valve::On(int curTime){
    
    this->gpioPin->On();
    vector<bool> curRes;
    curRes.push_back(true);
    this->valveRec->PushData((unsigned long)curTime,curRes);
   
}
void Valve::Off(int curTime){
    this->gpioPin->Off();
    //this->pin->Off();

    vector<bool> curRes;
    curRes.push_back(false);
    this->valveRec->PushData((unsigned long)curTime,curRes);

}

Valve::Valve(string name,string path, int valveId,int _valIdx)
{
    
    this->valIdx = _valIdx;
    this->name = name;
    
    this->gpioPin.reset(new Pin(valveId));
    
    this->valveRec=new Recorder<bool>(this->name,path,"time,"+this->name);
    
}

Valve::~Valve()
{
    
    if(this->dummy)
        delete this->valveRec;
    
}
string Valve::GetValveName(){
    return this->name;
}
int Valve::GetValIdx(){
    return this->valIdx;
}