#include "FSMachine.hpp"
FSMachine::FSMachine(/* args */)
{
    this->p1_idx=0;
    this->p2_idx=0;
    this->p3_idx=0;
    
    this->p5_idx=0;
    this->p6_idx=0;
    this->p7_idx=0;
    this->p8_idx=0;
}

FSMachine::~FSMachine()
{
    //just clear the current task
    unsigned long dummy1;
    unsigned long dummy2;
    unsigned long dummy3;
    unsigned long dummy4;
    unsigned long dummy5;
    unsigned long dummy6;
    unsigned long dummy7;
    this->GetPhaseTime(dummy1,dummy2,dummy3,dummy4,dummy5,dummy6,dummy7);
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







//Time based finite state machine
// Calculate the phase time from previous step
// Start from phase 4



void FSMachine::PushMea1(int *mea){
    this->LHipBuf1[this->curRow1]=mea[LHIPPOS];
    this->RHipBuf1[this->curRow1]=mea[RHIPPOS];
    this->LKneBuf1[this->curRow1]=mea[LKNEPOS];
    this->RKneBuf1[this->curRow1]=mea[RKNEPOS];
    this->LAnkBuf1[this->curRow1]=mea[LANKPOS];
    this->RAnkBuf1[this->curRow1]=mea[RANKPOS];
    
    this->curRow1++;
}
void FSMachine::PushMea2(int *mea){
    this->LHipBuf2[this->curRow2]=mea[LHIPPOS];
    this->RHipBuf2[this->curRow2]=mea[RHIPPOS];
    this->LKneBuf2[this->curRow2]=mea[LKNEPOS];
    this->RKneBuf2[this->curRow2]=mea[RKNEPOS];
    this->LAnkBuf2[this->curRow2]=mea[LANKPOS];
    this->RAnkBuf2[this->curRow2]=mea[RANKPOS];
    this->curRow2++;
}
void FSMachine::clearBuf(){
    //for now we don't clear the buffer since as long as we initialize the index, it should be fine
    this->curRow1 = 0;
    this->curRow2=0;
}
void FSMachine::GetPhaseTime(unsigned long &p1_t,unsigned long &p2_t,unsigned long &p3_t,unsigned long &p5_t,unsigned long &p6_t,unsigned long &p7_t,unsigned long &p8_t){
    while(!this->taskQue.empty()){
        this->taskQue.front()->join();
        delete this->taskQue.front();
        this->taskQue.pop();
    }
    //return each gait phase time, multiply 1000 since it would be used in nano sec
    //p4 is the starting point, so p4_t always =0
    p1_t = (this->curRow1+1+this->p1_idx)*MSEC;
    p2_t = (this->curRow1+1+this->p2_idx)*MSEC;
    p3_t = (this->curRow1+1+this->p3_idx)*MSEC;
    p5_t = this->p5_idx*MSEC;
    p6_t = this->p6_idx*MSEC;
    p7_t = this->p7_idx*MSEC;
    p8_t = this->p8_idx*MSEC;



}
void FSMachine:: StartP8(int *curMea){
    this->PushMea1(curMea);//here is the last time in this period we called PushMea1
    this->p8_idx = this->curRow1;
    //we don't need curRow1++, since later we will use curRow2
    this->taskQue.push(new std::thread(&FSMachine::FindP5_to_P7,this));
}
void FSMachine::FindP5_to_P7(){
    //P5 is the smallest RAnkPos
    int *p5_ptr = std::min_element(this->RAnkBuf1,this->RAnkBuf1+this->curRow1);
    this->p5_idx = p5_ptr - this->RAnkBuf1;

    //cannot get slope change easily, now just use 0.2 seconds later
    this->p6_idx = this->p5_idx+200;

    int *p7_ptr = std::min_element(this->RKneBuf1,this->RKneBuf1+this->curRow1);
    this->p7_idx = p7_ptr-this->RKneBuf1;

}
void FSMachine::FindP1_to_P3(){
    //P5 is the smallest RAnkPos
    int *p1_ptr = std::min_element(this->LAnkBuf2,this->LAnkBuf2+this->curRow2);
    this->p1_idx = p1_ptr - this->LAnkBuf2;
    //cannot get slope change easily, now just use 0.2 seconds later
    this->p2_idx = this->p1_idx+200;
    int *p3_ptr = std::min_element(this->LKneBuf2, this->LKneBuf2+this->curRow2);
    this->p7_idx = p3_ptr-this->LKneBuf2;

}