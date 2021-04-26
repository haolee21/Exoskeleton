#ifndef VALVE_H
#define VALVE_H
#include <string>
#include <mutex>
#include <Pin.hpp>
#include <chrono>
#include<ctime> //this timer
#include "Recorder.hpp"
#include <memory>
#include <iostream>
#include <string>



#define MAXRECLENGTHVAL 24000


class Valve
{
private:
    std::string name;
    
    std::shared_ptr<Pin> gpioPin;
    //shared_ptr<Pin> pin;
    
    chrono::system_clock::time_point startTime;
    void writeTempFile();
    bool recCond[MAXRECLENGTHVAL];
    int valIdx;//the index of this valve in valveCond array
    //unique_ptr<Recorder<bool>> valveRec;
    Recorder<bool,1> *valveRec;

    bool dummy=true;
public:
   
    Valve(std::string name,std::string path, int valveId, int valIdx);
    ~Valve();
    void On(int curTime);
    void Off(int curTime);
    int GetValIdx();
   
    std::string GetValveName();
    
};

#endif //VALVE_H
