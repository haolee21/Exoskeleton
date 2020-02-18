#ifndef FSMACHINE_HPP
#define FSMACHINE_HPP
#include "common.hpp"
#include <memory>
#include <algorithm>
#include <mutex> 
#include "MovAvgFilt.hpp"
#include <vector>
#include <Recorder.hpp>
#include <string>

#include <bits/stdc++.h> 
#include <functional>
#include <future>
// This is the finite state machine that used in the controller
// There are 8 phases in each gait

/*
    Left                                  Right 
1   Loading Response (Heel Strike)        Initial Swing 
2   Mid Stance                            Mid Swing
3   Terminal Stance     
4   Pre swing        (Ankle Push)         Terminal Swing
5   Initial Swing                         Loading Response (Heel Strike)
6   Mid Swing                             Mid Stance
7                                         Terminal Stance
8   Terminal Swing                        Pre swing (Ankle Push)

Individual Phase's control will be defined in Controller
FSM only tells what is the current state

*/
#define Phase1 0
#define Phase2 1
#define Phase3 2
#define Phase4 3
#define Phase5 4
#define Phase6 5
#define Phase7 6
#define Phase8 7



#define RHIP_PREP_POS 450

#define POS_BUF_SIZE 5000  //this is the upper limit of each phase, if one phase last for more than 2 sec, we should stop
#define MVFORDER 4//it has to be power of 2
typedef std::function<void()> actFun;
class FSMachine
{
private:
    //time based FSM
    int time;

    int p1_idx,p2_idx,p3_idx,p4_idx,p5_idx,p6_idx,p7_idx,p8_idx,p9_idx,p10_idx;
    int swIdx;
    int curIdx;


    int LHipBuf[POS_BUF_SIZE];
    int RHipBuf[POS_BUF_SIZE];
    int LKneBuf[POS_BUF_SIZE];
    int RKneBuf[POS_BUF_SIZE];
    int LAnkBuf[POS_BUF_SIZE];
    int RAnkBuf[POS_BUF_SIZE];




    // =============================================================================================
    void CalPhaseTime();
    void GetP1();
    void GetP2();
    void GetP3();
    void GetP4();
    void GetP5();
    void GetP6();
    void GetP7();
    void GetP8();
    void GetP9();
    void GetP10();

    //these values may not be constant 
    int LHipMean = 410;
    int RHipMean = 564;

    int preHipDiff;
    bool gaitStart;
    
    

    MovAvgFilt<MVFORDER> mvf;
    std::shared_ptr<Recorder<int>> FSMRec;


    //create the controller action when we reach each phase, these are lambda function passed down from the Controller
    std::function<void()> p1_act,p2_act,p3_act,p4_act,p5_act,p6_act,p7_act,p8_act,p9_act,p10_act;

    void _OneGait(); //this need to be fired in seperate thread
    void _OnePhase(std::function<void()> *actFun, int *time);
    struct timespec gaitTime;
    std::mutex gaitLock;
    bool isWalkFlag = false;
    std::future<void> curGait;
    //we need a seperate thread to fire the _OneGait


    void _GetPhaseTime(int curTime,long &p1_t,long &p2_t,long &p3_t,long &p4_t,long &p5_t,
                      long &p6_t, long &p7_t, long &p8_t, long &p9_t,long &p10_t);

    std::unique_ptr<Recorder<int>> InitPosRec;
    int initPosSetTime=0;
public:
    FSMachine(std::string filePath,actFun,actFun,actFun,actFun,actFun,actFun,actFun,actFun,actFun,actFun);
    ~FSMachine();
    void PushSen(int *curMea);
    
    void Reset();
   
    void SetInitPos(int curLHipPos,int curRHipPos);


};
#endif