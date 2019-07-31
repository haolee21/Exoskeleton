#ifndef CONTROLLER_H
#define CONTROLLER_H
#include "Valve.h"
#include <chrono>
#include<time.h> //this timer
#include<mutex>
#include <Recorder.hpp>
#include <memory>
#include <PWM.h>
#include <thread>
#include <memory>
#include "Displayer.hpp"
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
// index of command
#define TESTVAL 0
#define TESTPWM 1

struct Com
{
	const int comLen =2;
	bool comArray[2];
	mutex comLock;
};
const int VALNUM = 6; //this cannot work with test reacting
class Controller
{
private:
    int testSendCount; //test sending data, need to be removed

    std::shared_ptr<Valve> LKneVal1;
    std::shared_ptr<Valve> LKneVal2;
    std::shared_ptr<Valve> LAnkVal1;
    std::shared_ptr<Valve> LAnkVal2;
    std::shared_ptr<Valve> BalVal;
    std::shared_ptr<Valve> LRelVal;

    //displayer
    char *valveCond;
    bool display=false;
    //connect to pc
    std::shared_ptr<Displayer> client;  //shared_ptr doesn't work here since it cannot be automatically shutdown
    //Displayer *client;




    int *senData;

    std::shared_ptr<PWMGen> KnePreVal;
    std::shared_ptr<PWMGen> AnkPreVal;
    
    std::thread *knePreValTh;
    std::thread *ankPreValTh;

    std::shared_ptr<Recorder<int>> conRec;

    void WaitToSync();
    void Sleep(int sleepTime);

    void ValveOn(std::shared_ptr<Valve> val,int curTime);

    void ValveOff(std::shared_ptr<Valve> val,int curTime);
    //command
    Com *com;

    // valve control func and parameter
    // test reacting time
    struct TestReactParam
    {
        std::chrono::system_clock::time_point sendTime;
        bool dataNotSent = true;
        std::shared_ptr<Valve> testOut;
        // Valve *testOut; 
    };
    TestReactParam trParam; 
    void TestReactingTime();

    // test if valve is still functioning
    struct TestValParam
    {
        int testValIdx=0;
        int singleValCount =0;
        bool curValCond=false;
        const int maxTest = 20;
    };
    TestValParam tvParam;
    void TestValve();
    
    // test PWM Valve function
    struct TestPwmParam
    {
        bool notStart = true;
        int dutyLoopCount = 0;
        int curTestDuty=0;
    };
    TestPwmParam tpParam;
    void TestPWM();


public:
    // Valve* ValveList[VALNUM];
    std::shared_ptr<Valve> ValveList[VALNUM];

    Controller(std::string _filePath,Com *_com,bool display);
    ~Controller();
    
    
    void ConMainLoop(int *curSen,char* senRaw);
};



#endif //CONTROLLER_H