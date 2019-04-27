#ifndef CONTROLLER_H
#define CONTROLLER_H
#include "Valve.h"
#include <chrono>
#include<time.h> //this timer

//Define the pin number of the controller
// Attention, the pin number is different for c++ and python library
// Source: https://www.digikey.com/en/maker/blogs/2019/how-to-use-gpio-on-the-raspberry-pi-with-c
// BCM is for python, WiringPi is for c++
// I use common name: OP# to sync them
const int OP1 = 15;
const int OP2 = 16;
const int OP3 = 1;
const int OP4 = 4;
const int OP5 = 5;
const int OP6 = 6;
const int OP7 = 10;
const int OP8 = 26;
const int OP9 = 27;
const int OP10 = 28;
const int OP11 = 29;
const int OP12 = 8;
const int OP13 = 9;
const int OP14 = 0;
const int OP15 = 2;
const int OP16 = 3;


class Controller
{
private:
    /* data */
    int ValNum = 6;
    Valve LKneVal1 = Valve("LKneVal1",OP9);
    Valve LKneVal2 = Valve("LKneVal2",OP4);
    Valve LAnkVal1 = Valve("LAnkVal1",OP6);
    Valve LAnkVal2 = Valve("LAnkVal2",OP7);
    Valve BalVal = Valve("BalVal",OP3);
    Valve LRelVal = Valve("LRelVal",OP8);
    Valve ValveList[6]={LKneVal1,LKneVal2,LAnkVal1,LAnkVal2,BalVal,LRelVal};
    void WaitToSync();
    void Sleep(int sleepTime);
public:
    Controller();
    ~Controller();
    void TestValve();

};



#endif