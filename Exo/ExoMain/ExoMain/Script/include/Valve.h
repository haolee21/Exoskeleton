#ifndef VALVE_H
#define VALVE_H
#include <string>
#include <mutex>

#include <chrono>
#include<ctime> //this timer
#include "Recorder.hpp"
#include "Pin.hpp"
#include <memory>
#define MAXRECLENGTHVAL 24000
using namespace std;

class Valve
{
private:
    string name;
    int valveId;
    shared_ptr<Pin> pin;
    chrono::system_clock::time_point startTime;
    void writeTempFile();
    bool recCond[MAXRECLENGTHVAL];
    int valIdx;//the index of this valve in valveCond array
    //unique_ptr<Recorder<bool>> valveRec;
    Recorder<bool> *valveRec;

    bool dummy=true;
public:
   
    Valve(string name,string path, int valveId, int valIdx);
    ~Valve();
    void On(int curTime);
    void Off(int curTime);
    int GetValIdx();
   
    string GetValveName();
    
};

#endif //VALVE_H
