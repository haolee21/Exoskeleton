#include "Valve.h"
#include <iostream>
#include <wiringPi.h>
void Valve::On(){
    std::cout<<"valve on"<<std::endl;
    digitalWrite(this->valveId,HIGH);
}
void Valve::Off(){
    std::cout<<"valve off"<<std::endl;
    digitalWrite(this->valveId,LOW);
}
Valve::Valve(string name,int valveId)
{
    wiringPiSetup(); // This line is required everytime you setup an input output pin!!
    this->name = name;
    this->valveId = valveId;
    pinMode(valveId, OUTPUT);
    
}
Valve::~Valve()
{
    this->Off();
}