#include "FSMachine.hpp"
FSMachine::FSMachine(/* args */)
{
}

FSMachine::~FSMachine()
{
}



char FSMachine::CalState(int *curMea,char curState){
    
    char nextState;
    if(curState ==Phase1){
        if(true){
            nextState = Phase1;
        }
        else
        {
            nextState = Phase2;
        }
    }
    else if(curState == Phase2){
        

    }
    else if(curState==Phase3){

    }
    else if(curState == Phase4){

    }
    else if(curState==Phase5){

    }
    else if(curState==Phase6){

    }
    else if(curState == Phase7){

    }
    else if(curState==Phase8){

    }
  
    return nextState;
}