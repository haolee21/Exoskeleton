#ifndef FSMACHINE_HPP
#define FSMACHINE_HPP
#include "common.hpp"
#include <memory>
#include <algorithm>
#include <mutex> 
#include "MovAvgFilt.hpp"
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

#define POS_BUF_SIZE 10000  //this is the upper limit of each phase, if one phase last for more than 2 sec, we should stop
#define MVFORDER 2//it has to be power of 2
class FSMachine
{
private:
    //time based FSM
    unsigned int p1_idx;
    unsigned int p2_idx;
    unsigned int p3_idx;
    unsigned int p4_idx;
    unsigned int p5_idx;
    unsigned int p6_idx;
    unsigned int p7_idx;
    unsigned int p8_idx;
    unsigned int p9_idx;
    unsigned int p10_idx;
    unsigned int swIdx;
    unsigned int curIdx;
    // I know it looks redundant, but this makes searching minimal easier

    std::unique_ptr<int[]> LHipBuf;
    std::unique_ptr<int[]> RHipBuf;
    std::unique_ptr<int[]> LKneBuf;
    std::unique_ptr<int[]> RKneBuf;
    std::unique_ptr<int[]> LAnkBuf;
    std::unique_ptr<int[]> RAnkBuf;




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

public:
    FSMachine(/* args */);
    ~FSMachine();
    char CalState(int *curMea, char curState);
    void GetPhaseTime(unsigned long &p1_t, unsigned long &p2_t, unsigned long &p3_t,unsigned long &p4_t,unsigned long &p5_t,
                      unsigned long &p6_t, unsigned long &p7_t, unsigned long &p8_t, unsigned long &p9_t,unsigned long &p10_t);
    
    void PushSen(int *curMea);
    void LeftFront();
    void Reset();
    bool IsReady();
    void SetInitPos(int curLHipPos,int curRHipPos);
};
#endif