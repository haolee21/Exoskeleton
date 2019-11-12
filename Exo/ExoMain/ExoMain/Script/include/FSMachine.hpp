#ifndef FSMACHINE_HPP
#define FSMACHINE_HPP
#include "common.hpp"
#include <thread>
#include <queue>
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
#define NUM_OF_SAMP 3000

class FSMachine
{
private:
    int curRow1=0;
    int curRow2=0;

    //we need to define buffer for each joint individually, since if we do 2D array, we will have issue finding the idx by subtracting 2 pointers
    int LHipBuf1[NUM_OF_SAMP];
    int RHipBuf1[NUM_OF_SAMP];
    int LKneBuf1[NUM_OF_SAMP];
    int RKneBuf1[NUM_OF_SAMP];
    int LAnkBuf1[NUM_OF_SAMP];
    int RAnkBuf1[NUM_OF_SAMP];
    int LHipBuf2[NUM_OF_SAMP];
    int RHipBuf2[NUM_OF_SAMP];
    int LKneBuf2[NUM_OF_SAMP];
    int RKneBuf2[NUM_OF_SAMP];
    int LAnkBuf2[NUM_OF_SAMP];
    int RAnkBuf2[NUM_OF_SAMP];

    
    unsigned long p1_idx;
    unsigned long p2_idx;
    unsigned long p3_idx;
    
    unsigned long p5_idx;
    unsigned long p6_idx;
    unsigned long p7_idx;
    unsigned long p8_idx;
    void clearBuf();
    void FindP5_to_P7();
    void FindP1_to_P3();
    std::queue<std::thread*> taskQue; //use normal pointer here, not sure how to properly use unique_ptr in this case
public:
    FSMachine(/* args */);
    ~FSMachine();
    char CalState(int *curMea, char curState);
    void GetPhaseTime(unsigned long &p1_t,unsigned long &p2_t,unsigned long &p3_t,unsigned long &p5_t,unsigned long &p6_t,unsigned long &p7_t,unsigned long &p8_t);
    void PushMea1(int *mea);//push sen data into buf1 before reaching P8
    void StartP8(int *curMea);//push sen data when hit P8
    void PushMea2(int *mea);// push sen data into buf2 after reaching p8
    
};
#endif