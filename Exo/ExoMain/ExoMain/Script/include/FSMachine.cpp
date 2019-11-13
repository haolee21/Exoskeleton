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
void FSMachine::Reset(){
    this->curIdx1 = 0;
    this->curIdx2 = 0;
}
void FSMachine::GetPhaseTime(unsigned long &p1_t, unsigned long &p2_t, unsigned long &p3_t,
                      unsigned long &p5_t, unsigned long &p6_t, unsigned long &p7_t, unsigned long &p8_t)
{
    p1_t = (unsigned long)(this->p1_idx + this->curIdx1) * MSEC;
    p2_t = (unsigned long)(this->p2_idx + this->curIdx1) * MSEC;
    p3_t = (unsigned long)(this->p3_idx + this->curIdx1) * MSEC;
    p5_t = (unsigned long)this->p5_idx * MSEC;
    p6_t = (unsigned long)this->p6_idx * MSEC;
    p7_t = (unsigned long)this->p7_idx * MSEC;
    p8_t = (unsigned long)this->p8_idx * MSEC;



    this->Reset();

}
void FSMachine::GetP5(){
    int *swPoint = std::max_element(this->RAnkBuf1.get(), this->RAnkBuf1.get() + this->curIdx1);
    this->p5_idx = swPoint - this->RAnkBuf1.get();
}
void FSMachine::GetP1(){
    int *swPoint = std::min_element(this->LAnkBuf1.get(), this->LAnkBuf1.get() + this->curIdx1);
    this->p1_idx = swPoint - this->LAnkBuf1.get();
}
void FSMachine::GetP3(){
    int *swPoint = std::min_element(this->LKneBuf2.get(), this->LKneBuf2.get() + this->curIdx2);
    this->p3_idx = swPoint -this->LKneBuf2.get();
    // for here we hard code the ankle push-off time 
    // here we define it is in the mid of p1 and p3
    unsigned int p2 = this->p1_idx + this->p3_idx;
    this->p2_idx = p2 >> 1;
}
void FSMachine::GetP7(){
    int *swPoint = std::max_element(this->RKneBuf2.get(), this->RKneBuf2.get() + this->curIdx2);
    this->p7_idx = swPoint - this->RKneBuf2.get();
    // for here we hard code the ankle push-off time 
    // here we define it is in the mid of p5 and p7_idx
    unsigned int p6 = this->p5_idx + this->p7_idx;
    this->p6_idx = p6 >> 1;
}