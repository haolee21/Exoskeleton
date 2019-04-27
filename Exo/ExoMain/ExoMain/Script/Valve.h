#include <string>
#include<wiringPi.h>
using namespace std;
    

class Valve
{
private:
    string name;
    int valveId;


public:
    Valve(string name, int valveId);
    ~Valve();
    void On();
    void Off();
};


