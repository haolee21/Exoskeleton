#ifndef VALVE_H
#define VALVE_H
#include <string>
#include <mutex>

#include <chrono>
#include<ctime> //this timer

#define MAXRECLENGTH 24000
using namespace std;

class Valve
{
private:
    string name;
    int valveId;
    
    chrono::system_clock::time_point startTime;
    void writeTempFile();
    bool recCond[MAXRECLENGTH];
    int curRecIndex;
public:
    Valve(string name, int valveId);
    ~Valve();
    void On();
    void Off();
    
    void WriteWholeFile();
    // Time is recorded under raspberry pi's timer, need to be sync with arduino later
    // We record the time difference 
    // The size of the array is too big to only use the memory on stack, we must allocate it on heap
    // Need to delete the array when valve object is destoried
    //double *valTimeRec = new double[recLength];
    //int recIndex;
    //bool *valCondRec=new bool[recLength];
};

#endif //VALVE_H
