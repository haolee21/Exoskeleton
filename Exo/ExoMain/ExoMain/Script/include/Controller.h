#ifndef CONTROLLER_H
#define CONTROLLER_H
#include "common.hpp"
#include "FSMachine.hpp"
#include "ValveCon.hpp"
#include "PIDCon.h"
#include <chrono>
#include<time.h> //this timer
#include<mutex>
#include "Recorder.hpp"
#include <memory>
#include <thread>
#include <memory>
#include <iomanip>
#include "Displayer.hpp"
#include <iostream>
#include <queue>
#include "MovAvgFilt.hpp"




// index of command
#define NUMCOM 28
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

#define FSM_ADV 27
struct Com
{
	const int comLen =NUMCOM;
	bool comArray[NUMCOM];
	
    int comVal[NUMCOM];//if any value need to be passed
    bool offArray[NUMCOM]; //If controller will self turn off the function, it will set the offArray as true, and required actions will be taken
};
// index of senData
// #define NUMSEN 16







class Controller
{
private:
    
    

    std::mutex *senLock;
    std::mutex conSW_lock;
    bool sw_conMain = false;
    std::thread conMain_th;
    
    int *encData;
    int *adcData;


    long sampT;
    void WaitToSync();
    void Sleep(int sleepTime);

    



    std::unique_ptr<ValveCon> valveCon;
    std::chrono::system_clock::time_point t_origin;
    //command

    Com *com;

    
    // valve control func and parameter


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
    int ankActPre = 300;
    int kneSupPre = 300;
    int ankSupPre = 300;
    int kneRecPre = 250;
    int ankRecPre = 350;

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
    void AnkPushOff_main(int preVal_id, int ankPre, int tankPre);
    float AnkActInput(int ankPre,int tankPre);
    
    
    
    float KnePreRecInput(int knePre,int tankPre);
    bool CheckKnePreRec_main(int knePreVal_id,int knePre, int tankPre,int desPre);

   
    float AnkPreRecInput(int ankPre,int tankPre);
    bool CheckAnkPreRec_main(int ankPreVal_id, int ankPre, int tankPre, int balVal_id,int desPre);


    
    bool CheckSupPre_main(int preVal_id, int knePre, int tankPre, int supPre);
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
    

    Controller(std::string _filePath,Com *_com,std::chrono::system_clock::time_point origin,long _sampT
                , int *encData, int *adcData,std::mutex *senDataLock);
    ~Controller();
    void PreRel();

    void Start();
    void Stop();
    void ConMainLoop();
};




#endif //CONTROLLER_H