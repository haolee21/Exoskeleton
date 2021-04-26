#include "Valve.h"
#include <array>
using namespace std;
void Valve::On(int curTime){
    
    this->gpioPin->On();

    array<bool,1> curRes={true};
    this->valveRec->PushData((unsigned long)curTime,curRes);
   
}
void Valve::Off(int curTime){
    this->gpioPin->Off();
    //this->pin->Off();
    array<bool,1> curRes = {false};
    this->valveRec->PushData((unsigned long)curTime,curRes);

}

Valve::Valve(string name,string path, int valveId,int _valIdx)
{
    
    this->valIdx = _valIdx;
    this->name = name;
    
    this->gpioPin.reset(new Pin(valveId));
    
    this->valveRec=new Recorder<bool,1>(this->name,path,"time,"+this->name);
    
}

Valve::~Valve()
{
    this->gpioPin->Off();

    if(this->dummy)
        delete this->valveRec;
    
}
string Valve::GetValveName(){
    return this->name;
}
int Valve::GetValIdx(){
    return this->valIdx;
}