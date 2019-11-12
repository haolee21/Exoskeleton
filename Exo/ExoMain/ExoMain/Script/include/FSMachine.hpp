#ifndef FSMACHINE_HPP
#define FSMACHINE_HPP
#include "common.hpp"
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

class FSMachine
{
private:
    /* data */
public:
    FSMachine(/* args */);
    ~FSMachine();
    char CalState(int *curMea, char curState);
};
#endif