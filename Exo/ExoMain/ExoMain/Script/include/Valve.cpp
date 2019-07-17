#include "Valve.h"
#include <iostream>
#include <string>
#include <wiringPi.h>
#include <chrono>
#include<ctime> //this timer
void Valve::On(){
    std::chrono::system_clock::time_point curTime = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed = curTime - this->startTime;
    //std::cout<<"valve on"<<std::endl;
    digitalWrite(this->valveId,HIGH);
    this->recCond[this->curRecIndex++] = true;
    // this->valCondRec[this->recIndex] = true;
    // this->valTimeRec[this->recIndex]=elapsed.count();
    // this->recIndex++;
}
void Valve::Off(){
    std::chrono::system_clock::time_point curTime = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed = curTime - this->startTime;
    //std::cout<<"valve off"<<std::endl;
    digitalWrite(this->valveId,LOW);
    this->recCond[this->curRecIndex++] = false;
    // this->valCondRec[this->recIndex] = false;
    // this->valTimeRec[this->recIndex] = elapsed.count();
    // this->recIndex++;
}
Valve::Valve(string name,int valveId)
{
    wiringPiSetup(); // This line is required everytime you setup an input output pin!!
    this->name = name;
    this->valveId = valveId;
    pinMode(valveId, OUTPUT);
    this->curRecIndex = 0;
    this->recCond[this->curRecIndex++] = false;
}

Valve::~Valve()
{
    // delete[] this->valTimeRec;
    // delete[] this->valCondRec;
}