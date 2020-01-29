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
class FSMachine
{
private:
    //time based FSM
    int time;

    int p1_idx;
    int p2_idx;
    int p3_idx;
    int p4_idx;
    int p5_idx;
    int p6_idx;
    int p7_idx;
    int p8_idx;
    int p9_idx;
    int p10_idx;
    int swIdx;
    int curIdx;
    // I know it looks redundant, but this makes searching minimal easier

    int LHipBuf[POS_BUF_SIZE];
    int RHipBuf[POS_BUF_SIZE];
    int LKneBuf[POS_BUF_SIZE];
    int RKneBuf[POS_BUF_SIZE];
    int LAnkBuf[POS_BUF_SIZE];
    int RAnkBuf[POS_BUF_SIZE];

    // ============ this is for verify the correctness of FSM phase time calculation ===============
    std::unique_ptr<Recorder<int>> LHipRec;
    std::unique_ptr<Recorder<int>> RHipRec;
    std::unique_ptr<Recorder<int>> LKneRec;
    std::unique_ptr<Recorder<int>> RKneRec;
    std::unique_ptr<Recorder<int>> LAnkRec;
    std::unique_ptr<Recorder<int>> RAnkRec;
    bool recInGait=false; //this flag is for making all measurements into one vector for each gait
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
    bool halfGait;
    bool timeReady;
    std::mutex lock;

    MovAvgFilt<MVFORDER> mvf;
    std::shared_ptr<Recorder<int>> FSMRec;

public:
    FSMachine(std::string filePath);
    ~FSMachine();
    char CalState(int *curMea, char curState);
    void GetPhaseTime(int curTime,long &p1_t,long &p2_t,long &p3_t,long &p4_t,long &p5_t,
                      long &p6_t, long &p7_t, long &p8_t, long &p9_t,long &p10_t);
    
    void PushSen(int *curMea);
    void LeftFront();
    void Reset();
    bool IsReady();
    void SetInitPos(int curLHipPos,int curRHipPos);
};
#endif