#ifndef CONTROLLER_H
#define CONTROLLER_H
#include "Valve.h"
#include <chrono>
#include<time.h> //this timer


//Define the pin number of the controller
// Attention, the pin number is different for c++ and python library
// Source: https://www.digikey.com/en/maker/blogs/2019/how-to-use-gpio-on-the-raspberry-pi-with-c
// BCM is for python, WiringPi is for c++
//  +-----+-----+---------+------+---+---Pi 3---+---+------+---------+-----+-----+
//  | BCM | wPi |   Name  | Mode | V | Physical | V | Mode | Name    | wPi | BCM |
//  +-----+-----+---------+------+---+----++----+---+------+---------+-----+-----+
//  |     |     |    3.3v |      |   |  1 || 2  |   |      | 5v      |     |     |
//  |   2 |   8 |   SDA.1 |   IN | 1 |  3 || 4  |   |      | 5v      |     |     |
//  |   3 |   9 |   SCL.1 |   IN | 1 |  5 || 6  |   |      | 0v      |     |     |
//  |   4 |   7 | GPIO. 7 |   IN | 1 |  7 || 8  | 1 | ALT5 | TxD     | 15  | 14  |
//  |     |     |      0v |      |   |  9 || 10 | 1 | ALT5 | RxD     | 16  | 15  |
//  |  17 |   0 | GPIO. 0 |   IN | 0 | 11 || 12 | 0 | IN   | GPIO. 1 | 1   | 18  |
//  |  27 |   2 | GPIO. 2 |   IN | 0 | 13 || 14 |   |      | 0v      |     |     |
//  |  22 |   3 | GPIO. 3 |   IN | 0 | 15 || 16 | 0 | IN   | GPIO. 4 | 4   | 23  |
//  |     |     |    3.3v |      |   | 17 || 18 | 0 | IN   | GPIO. 5 | 5   | 24  |
//  |  10 |  12 |    MOSI |   IN | 0 | 19 || 20 |   |      | 0v      |     |     |
//  |   9 |  13 |    MISO |   IN | 0 | 21 || 22 | 0 | IN   | GPIO. 6 | 6   | 25  |
//  |  11 |  14 |    SCLK |   IN | 0 | 23 || 24 | 1 | IN   | CE0     | 10  | 8   |
//  |     |     |      0v |      |   | 25 || 26 | 1 | IN   | CE1     | 11  | 7   |
//  |   0 |  30 |   SDA.0 |   IN | 1 | 27 || 28 | 1 | IN   | SCL.0   | 31  | 1   |
//  |   5 |  21 | GPIO.21 |   IN | 1 | 29 || 30 |   |      | 0v      |     |     |
//  |   6 |  22 | GPIO.22 |   IN | 1 | 31 || 32 | 0 | IN   | GPIO.26 | 26  | 12  |
//  |  13 |  23 | GPIO.23 |   IN | 0 | 33 || 34 |   |      | 0v      |     |     |
//  |  19 |  24 | GPIO.24 |   IN | 0 | 35 || 36 | 0 | IN   | GPIO.27 | 27  | 16  |
//  |  26 |  25 | GPIO.25 |   IN | 0 | 37 || 38 | 0 | IN   | GPIO.28 | 28  | 20  |
//  |     |     |      0v |      |   | 39 || 40 | 0 | IN   | GPIO.29 | 29  | 21  |
//  +-----+-----+---------+------+---+----++----+---+------+---------+-----+-----+
//  | BCM | wPi |   Name  | Mode | V | Physical | V | Mode | Name    | wPi | BCM |
//  +-----+-----+---------+------+---+---Pi 3---+---+------+---------+-----+-----+

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
    Valve BalVal = Valve("BalVal",OP10);
    Valve LRelVal = Valve("LRelVal",OP8);
    
    Valve testOut = Valve("TestMea",8); //this uses gpio2


 

    void WaitToSync();
    void Sleep(int sleepTime);
    void Wait(long waitMilli);
    int preTime; //this is for testing sen
public:
    Valve ValveList[6]={LKneVal1,LKneVal2,LAnkVal1,LAnkVal2,BalVal,LRelVal};
    Controller();
    ~Controller();
    void TestValve();
    bool SendTestMeasurement(bool sendState);
    bool WaitTestMeasurement(std::chrono::system_clock::time_point &senseTime,bool &testState,int *senData);
    void ConMainLoop(int *senData);
};



#endif //CONTROLLER_H