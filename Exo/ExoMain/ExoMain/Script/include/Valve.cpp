#include "Valve.h"
#include <iostream>
#include <string>
#include <wiringPi.h>
#include <chrono>
#include<ctime> //this timer
void Valve::On(int curTime){
    
  
    digitalWrite(this->valveId,HIGH);
    vector<bool> curRes;
    curRes.push_back(true);
    this->valveRec->PushData((unsigned long)curTime,curRes);
   
}
void Valve::Off(int curTime){
    
  
    digitalWrite(this->valveId,LOW);
    vector<bool> curRes;
    curRes.push_back(false);
    this->valveRec->PushData((unsigned long)curTime,curRes);

}
Valve::Valve(string name,string path, int valveId,int _valIdx)
{
    wiringPiSetup(); // This line is required everytime you setup an input output pin!!
    this->valIdx = _valIdx;
    this->name = name;
    this->valveId = valveId;
    pinMode(valveId, OUTPUT);
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