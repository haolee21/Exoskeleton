#include <string>
#include <mutex>
#include "Sensor.h"
#include <chrono>
#include<ctime> //this timer
using namespace std;
const int recLength = 240000; //default data length for valve, can record in 100 Hz for 30 min
class Valve
{
private:
    string name;
    int valveId;
    
    chrono::system_clock::time_point startTime;
    

public:
    Valve(string name, int valveId);
    ~Valve();
    void On();
    void Off();
    void SetStartTime( chrono::system_clock::time_point startTime);
    // Time is recorded under raspberry pi's timer, need to be sync with arduino later
    // We record the time difference 
    // The size of the array is too big to only use the memory on stack, we must allocate it on heap
    // Need to delete the array when valve object is destoried
    double *valTimeRec = new double[recLength];
    int recIndex;
    bool *valCondRec=new bool[recLength];
};


