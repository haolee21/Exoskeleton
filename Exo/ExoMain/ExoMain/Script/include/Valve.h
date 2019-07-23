#ifndef VALVE_H
#define VALVE_H
#include <string>
#include <mutex>

#include <chrono>
#include<ctime> //this timer
#include "Recorder.hpp"
#include <memory>
#define MAXRECLENGTHVAL 24000
using namespace std;

class Valve
{
private:
    string name;
    int valveId;
    
    chrono::system_clock::time_point startTime;
    void writeTempFile();
    bool recCond[MAXRECLENGTHVAL];
    int curRecIndex;
    //unique_ptr<Recorder<bool>> valveRec;
    Recorder<bool> *valveRec;

    bool dummy=true;
public:
    Valve(string name, int valveId);
    Valve(string name, int valveId, bool _dummy);
    ~Valve();
    void On(int curTime);
    void Off(int curTime);
    
   
    string GetValveName();
    
};

#endif //VALVE_H
