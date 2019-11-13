#include "FSMachine.hpp"
FSMachine::FSMachine(/* args */)
{
    this->LHipBuf1.reset(new int[POS_BUF_SIZE]);
    this->RHipBuf1.reset(new int[POS_BUF_SIZE]);
    this->LKneBuf1.reset(new int[POS_BUF_SIZE]);
    this->RKneBuf1.reset(new int[POS_BUF_SIZE]);
    this->LAnkBuf1.reset(new int[POS_BUF_SIZE]);
    this->RAnkBuf1.reset(new int[POS_BUF_SIZE]);

    this->LHipBuf2.reset(new int[POS_BUF_SIZE]);
    this->RHipBuf2.reset(new int[POS_BUF_SIZE]);
    this->LKneBuf2.reset(new int[POS_BUF_SIZE]);
    this->RKneBuf2.reset(new int[POS_BUF_SIZE]);
    this->LAnkBuf2.reset(new int[POS_BUF_SIZE]);
    this->RAnkBuf2.reset(new int[POS_BUF_SIZE]);
}

FSMachine::~FSMachine()
{
}
#define LKNE_MIN_POS 150
#define LHIP_MIN_POS 230
#define LANK_PUSH_POS 280
#define RKNE_REC_POS 600
#define RKNE_ENDREC_POS 690
#define RHIP_PUSH_POS 600
#define RANK_ENDPUSH_POS 450
#define LKNE_REC_POS 690
char FSMachine::CalState(int *curMea,char curState){
    
    

    switch (curState){
        case Phase1:
            if(curMea[LKNEPOS]>LKNE_MIN_POS){
                return Phase1;
                
            }
            else{
                std::cout<<"Phase2\n";
                return Phase2;
                
            }
            break;
        case Phase2:
            if(curMea[LHIPPOS]>LHIP_MIN_POS){
                return Phase2;
            }
            else{
                std::cout<<"Phase3\n";
                return  Phase3;
                
            }
            break;
        case Phase3:
            if(curMea[LANKPOS]>LANK_PUSH_POS){
                return  Phase3;
            }
            else{
                std::cout<<"Phase4\n";
                return  Phase4;
                
            }
            break;
        case Phase4:
            if(curMea[RKNEPOS]<RKNE_REC_POS){
                return  Phase4;
            }
            else{
                std::cout<<"Phase5\n";
                return  Phase5;
                
            }
            break;
        case Phase5:
            if(curMea[RKNEPOS]<RKNE_ENDREC_POS){
                return  Phase5;
            }
            else{
                std::cout<<"Phase6\n";
                return  Phase6;
                
            }
            break;
        case Phase6:
            if(curMea[RHIPPOS]<RHIP_PUSH_POS){
                return  Phase6;
            }
            else{
                std::cout<<"Phase7\n";
                return  Phase7;
                
            }
            break;
        case Phase7:
            if(curMea[RANKPOS]<RANK_ENDPUSH_POS){
                return  Phase7;
            }
            else{
                std::cout<<"Phase8\n";
                return  Phase8;
            }
            break;
        case Phase8:
            if(curMea[LKNEPOS]<LKNE_REC_POS){
                return  Phase8;
            }
            else{
                std::cout<<"Phase1\n";
                return  Phase1;
            }
            break;
        default:
            return curState;
    }
    
}

//time based FSM
void FSMachine::PushSen1(int *curMea){
    // if it reaches max size of the buffer, we stop recording
    if(this->curIdx1<POS_BUF_SIZE){
        this->LHipBuf1[this->curIdx1] = curMea[LHIPPOS];
        this->RHipBuf1[this->curIdx1] = curMea[RHIPPOS];
        this->LKneBuf1[this->curIdx1] = curMea[LKNEPOS];
        this->RKneBuf1[this->curIdx1] = curMea[RKNEPOS];
        this->LAnkBuf1[this->curIdx1] = curMea[LANKPOS];
        this->RAnkBuf1[this->curIdx1] = curMea[RANKPOS];
    }
}
void FSMachine::PushSen2(int *curMea){
    if(this->curIdx2<POS_BUF_SIZE){
        this->LHipBuf2[this->curIdx2] = curMea[LHIPPOS];
        this->RHipBuf2[this->curIdx2] = curMea[RHIPPOS];
        this->LKneBuf2[this->curIdx2] = curMea[LKNEPOS];
        this->RKneBuf2[this->curIdx2] = curMea[RKNEPOS];
        this->LAnkBuf2[this->curIdx2] = curMea[LANKPOS];
        this->RAnkBuf2[this->curIdx2] = curMea[RANKPOS];
    }
}
void FSMachine::PushSen(int *curMea){
    if(this->reachP8){
        this->PushSen2(curMea);
    }
    else{
        this->PushSen1(curMea);
    }
}
void FSMachine::ReachP8(){
    this->reachP8 = true;
}