#ifndef FSMACHINE_HPP
#define FSMACHINE_HPP
#include "common.hpp"
#include <memory>
#include <algorithm>
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
class FSMachine
{
private:
    //time based FSM
    unsigned int p1_idx;
    unsigned int p2_idx;
    unsigned int p3_idx;
    unsigned int p5_idx;
    unsigned int p6_idx;
    unsigned int p7_idx;
    unsigned int p8_idx;
    unsigned int curIdx1;
    unsigned int curIdx2;
    // I know it looks redundant, but this makes searching minimal easier

    std::unique_ptr<int[]> LHipBuf1;
    std::unique_ptr<int[]> RHipBuf1;
    std::unique_ptr<int[]> LKneBuf1;
    std::unique_ptr<int[]> RKneBuf1;
    std::unique_ptr<int[]> LAnkBuf1;
    std::unique_ptr<int[]> RAnkBuf1;

    std::unique_ptr<int[]> LHipBuf2;
    std::unique_ptr<int[]> RHipBuf2;
    std::unique_ptr<int[]> LKneBuf2;
    std::unique_ptr<int[]> RKneBuf2;
    std::unique_ptr<int[]> LAnkBuf2;
    std::unique_ptr<int[]> RAnkBuf2;
    void PushSen1(int *curMea);
    void PushSen2(int *curMea);
    bool reachP8 = false;
    //P1 and P5 search min angle of ankle
    void GetP5();
    void GetP1();
    //P3 and P7 search min angle of knee
    //phase detection functions
    void GetP3();
    void GetP7();

    bool gaitEnd;

public:
    FSMachine(/* args */);
    ~FSMachine();
    char CalState(int *curMea, char curState);
    void GetPhaseTime(unsigned long &p1_t, unsigned long &p2_t, unsigned long &p3_t,
                      unsigned long &p5_t, unsigned long &p6_t, unsigned long &p7_t, unsigned long &p8_t);
    
    void PushSen(int *curMea);
    void ReachP8();
    void Reset();
};
#endif