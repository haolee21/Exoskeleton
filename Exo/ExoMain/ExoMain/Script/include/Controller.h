#ifndef CONTROLLER_H
#define CONTROLLER_H
#include "common.hpp"
#include "FSMachine.hpp"
#include "Valve.h"
#include "PIDCon.h"
#include <chrono>
#include<time.h> //this timer
#include<mutex>
#include <Recorder.hpp>
#include <memory>
#include <PWM.h>
#include <thread>
#include <memory>
#include <iomanip>
#include "Displayer.hpp"
#include <iostream>
#include <queue>
#include "MovAvgFilt.hpp"
// new modification: After having issues with wiringPi, I no longer use this library (not compatiable with cross-compiler for no reason)
// Now all the pin number is BCM, same as python

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
// #define OP1  15  14 
// #define OP2  16  15
// #define OP3   1  18
// #define OP4   4  23
// #define OP5   5  24
// #define OP6   6  25
// #define OP7  10  8
// #define OP8  26  12
// #define OP9  27  16
// #define OP10 28  20
// #define OP11 29  21
// #define OP12  8  2
// #define OP13  9  3
// #define OP14  0  17
// #define OP15  2  27
// #define OP16  3  22
// #define SYNCOUT 7 4

#define OP1  14 
#define OP2  15
#define OP3  18
#define OP4  23
#define OP5  24
#define OP6  25
#define OP7  8
#define OP8  12
#define OP9  16
#define OP10 20
#define OP11 21
#define OP12 2
#define OP13 3
#define OP14 17
#define OP15 27
#define OP16 22
#define SYNCOUT 4

// index of command
#define NUMCOM 27
#define TESTVAL 0
#define TESTPWM 1
#define SHUTPWM 2

#define KNEMODSAMP 3
#define KNEPREREL 4
#define TESTALLLEAK 5
#define FREEWALK 6
#define TESTLEAK 7
#define TESTLANK 8
#define TESTRANK 9
#define SHOWSEN 10 //Cout the current sensor measurements, 
#define BIPEDREC 11
#define TESTSYNC 12
#define PIDACTTEST 13
#define PIDRECTEST 14
#define TESTONEPWM 15

#define CON_LANK_ACT 16
#define CON_RANK_ACT 17
#define CON_LKNE_REC 18
#define CON_RKNE_REC 19
#define CON_LANK_REC 20
#define CON_RANK_REC 21
#define CON_LKNE_SUP 22
#define CON_RKNE_SUP 23
#define CON_LANK_SUP 24
#define CON_RANK_SUP 25
#define CON_SET_INIT_POS 26
struct Com
{
	const int comLen =NUMCOM;
	bool comArray[NUMCOM];
	mutex comLock;
    int comVal[NUMCOM];//if any value need to be passed
    bool offArray[NUMCOM]; //If controller will self turn off the function, it will set the offArray as true, and required actions will be taken
};
// index of senData
// #define NUMSEN 16







class Controller
{
private:
    int testSendCount; //test sending data, need to be removed

    std::shared_ptr<Valve> LKneVal; 
    std::shared_ptr<Valve> RKneVal;
    std::shared_ptr<Valve> LAnkVal;
    std::shared_ptr<Valve> RAnkVal;
    
    std::shared_ptr<Valve> LBalVal;
    std::shared_ptr<Valve> RBalVal;
    std::shared_ptr<Valve> RelVal;

    
    //connect to pc
    std::shared_ptr<Displayer> client;  //shared_ptr doesn't work here since it cannot be automatically shutdown
    //Displayer *client;
    void SendToDisp(const char *senRaw);

    std::mutex *senLock;
    std::mutex conSW_lock;
    bool sw_conMain = false;
    std::thread conMain_th;
    //shared_ptr<int> senData;
    int senData[NUMSEN+1];
    int preSen[NUMSEN+1]; 

    std::shared_ptr<PWMGen> LKnePreVal;
    std::shared_ptr<PWMGen> RKnePreVal;
    
    std::shared_ptr<PWMGen> LAnkPreVal;
    std::shared_ptr<PWMGen> RAnkPreVal;
    std::shared_ptr<PWMGen> PWMList[PWMNUM];

    long sampT;
    void WaitToSync();
    void Sleep(int sleepTime);

    //displayer
    // char *valveCond;
    std::mutex ValveCondLock; //protect pwmDuty and ValveCond since I will have to update it in FSMLoop
    std::shared_ptr<char[]> valveCond;

    bool display=false;
    int preSend=0; //scale the sending freq since matplotlib cannot handle it
    const int dispPreScale = 39; //determine how frequent we send data back to pc, 1000/40=25 Hz
    // Valve control
    void ValveOn(std::shared_ptr<Valve> val);
    void ValveOff(std::shared_ptr<Valve> val);
    // PWM control
    void SetDuty(std::shared_ptr<PWMGen> pwmVal,int duty);
    // char *pwmDuty;
    std::shared_ptr<char[]> pwmDuty;
    //command

    Com *com;

    // valve control func and parameter
    // test reacting time
    struct TestReactParam
    {
        std::chrono::system_clock::time_point sendTime;
        bool dataNotSent = true;
        std::shared_ptr<Valve> testOut; 
        
    };
    TestReactParam trParam; 
    void TestReactingTime();

    // test if valve is still functioning
    struct TestValParam
    {
        int testValIdx=0;
        int singleValCount =0;
        bool curValCond=true;
        const int maxTest = 20;
        int curTestCount=0; //valve cannot operate in such high freq, need to prescale
        const int maxTestCount=12;
    };
    TestValParam tvParam;
    void TestValve();
    
    // test PWM Valve function
    struct TestPwmParam
    {
        int testPWMidx=0;
        bool notStart = true;
        int dutyLoopCount = 0;
        int curTestDuty=0;
        const int preScaler = 100;
    };
    TestPwmParam tpParam;
    void TestPWM();
    void ShutDownPWM();

    //test single pwm valve
    // valve will be actuate with 33% duty cycle
    std::shared_ptr<PWMGen> testPWM;
    bool testOnePWM_flag = false;
    void TestOnePWM(int pwmIdx);


    // initialize cylinder for supporting body weight
    int sup_LKnePre=250;
    // left leg energy recycle
    struct LeftEngRecycle
    {
        int curPhase=1;
    };
    LeftEngRecycle LEngRec;
    int CalDuty(int curPre, int desPre,int tankPre);
    

    
    //sample Model data 
    struct ConModSamp{
        const int maxCycle=5;
        int cycleCount=0;
        int outLoopCount=0;
        const int maxOuterLoop=299;
    };
    ConModSamp sampKneMod;
    void SampKneMod(int testDuty);
    struct KneePressureRelease{
        const int maxRelCycle=50;
        int curRelCycle=0;

    };
    KneePressureRelease knePreRel;
    void KneRel(); //release the pressure of knee joint
    //


    // Testing if there is any leakage in the loop
    struct TestLeakPara{
        int curPath;
        int curCount=0;
        const int maxCount = 1000;
        bool all_on = false;
        
    };
    TestLeakPara testLeak;
    void TestLeak(int curPath);
    void TestAllLeak();
    void TestAllLeakOn();
    void TestAllLeakOff();

    //Free walk
    bool freeWalk_on = false;
    void FreeWalk();
    void FreeWalk_on();
    void FreeWalk_off();


    //PID controller test
    //=========================================================================================================================
    void PIDActTest(int joint);
    void PIDRecTest(int joint);






    //=========================================================================================================================
    //PID Controllers
    // std::shared_ptr<PIDCon> kneRecPID;
    // std::shared_ptr<PIDCon> ankRecPID;
    // std::shared_ptr<PIDCon> kneSupPID; // we need to re-assign a PID controller everytime we went into that stage
    //                                       // the controller are created when phase_pre => phase_next if it is going to be used in next phase
    //                                       // we only need one since we will need to prepare for impact on one leg only
    // std::shared_ptr<PIDCon> ankActPID;

    //flag for make sure we have update the pid controller parameters
    bool kneRecPID_set = false;
    bool ankRecPID_set = false;
    bool kneSupPID_set = false;
    bool ankActPID_set = false;

    //=========================================================================================================================
    //Biped walking energy recycle
    std::shared_ptr<FSMachine> FSM;
    // FSMachine FSM;
    char curState;
    int ankActPre = 320;
    int kneSupPre = 350;
    int ankSupPre = 300;
    int kneRecPre = 250;
    int ankRecPre = 250;

    int LHipMean = 410;
    int RHipMean = 564;

    void BipedEngRec();
    void Init_swing(char side);
    void Mid_swing(char side);
    void Term_swing(char side);
    void Load_resp(char side);
    void Mid_stance(char side);
    void Term_stance(char side);
    void Pre_swing(char side);

    // since measurements will not be updated in FSM, we need to call a different function in controller's main loop
    void AnkPushOff_main(std::shared_ptr<PWMGen> preVal, int ankPre, int tankPre);
    float AnkActInput(int ankPre,int tankPre);
    
    
    
    float KnePreRecInput(int knePre,int tankPre);
    bool CheckKnePreRec_main(std::shared_ptr<PWMGen> knePreVal,int knePre, int tankPre,int desPre);

   
    float AnkPreRecInput(int ankPre,int tankPre);
    bool CheckAnkPreRec_main(std::shared_ptr<PWMGen> ankPreVal, int ankPre, int tankPre, std::shared_ptr<Valve> balVal,int desPre);


    
    bool CheckSupPre_main(std::shared_ptr<PWMGen> preVal, int knePre, int tankPre, int supPre);
    float SupPreInput(int knePre,int tankPre,int supPre);
    
    
    //time based FSM
    
    //============================================================================================================================================

    //Test ankle actuation
    bool TestRAnkFlag = false;
    bool TestLAnkFlag = false;
    void TestRAnk();
    void TestLAnk();


    //Print out the current measurements
    void ShowSen();
public:
    // Valve* ValveList[VALNUM];
    std::shared_ptr<Valve> ValveList[VALNUM];

    Controller(std::string _filePath,Com *_com,bool display,std::chrono::system_clock::time_point origin,long _sampT);
    ~Controller();
    void PreRel();

    void Start(int *senData, char *senRaw, std::mutex *senDataLock);
    void Stop();
    void ConMainLoop(int *curSen, char *senRaw,std::mutex *senDataLock);
};




#endif //CONTROLLER_H