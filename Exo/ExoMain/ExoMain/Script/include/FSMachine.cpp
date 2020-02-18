#include "FSMachine.hpp"

FSMachine::FSMachine(std::string filePath, actFun _p1_act,actFun _p2_act,actFun _p3_act,actFun _p4_act,
                        actFun _p5_act,actFun _p6_act,actFun _p7_act,actFun _p8_act,actFun _p9_act,actFun _p10_act)
{
    // this->LHipBuf.reset(new int[POS_BUF_SIZE]);
    // this->RHipBuf.reset(new int[POS_BUF_SIZE]);
    // this->LKneBuf.reset(new int[POS_BUF_SIZE]);
    // this->RKneBuf.reset(new int[POS_BUF_SIZE]);
    // this->LAnkBuf.reset(new int[POS_BUF_SIZE]);
    // this->RAnkBuf.reset(new int[POS_BUF_SIZE]);

    this->p1_idx = 0;
    this->p2_idx = 0;
    this->p3_idx = 0;
    this->p4_idx = 0;
    this->p5_idx = 0;
    this->p6_idx = 0;
    this->p7_idx = 0;
    this->p8_idx = 0;
    this->p9_idx = 0;
    this->p10_idx = 0;
    this->swIdx = 0;

    this->curIdx = 0;
    this->gaitStart = false;
    this->FSMRec.reset(new Recorder<int>("FSM", filePath, "time,phase1,phase2,phase3,phase4,phase5,phase6,phase7,phase8,phase9,phase10"));
    this->InitPosRec.reset(new Recorder<int>("FSMInitPos", filePath, "time,LHipPos,RHipPos"));
    //define the FSM act function
    this->p1_act = _p1_act;
    this->p2_act = _p2_act;
    this->p3_act = _p3_act;
    this->p4_act = _p4_act;
    this->p5_act = _p5_act;
    this->p6_act = _p6_act;
    this->p7_act = _p7_act;
    this->p8_act = _p8_act;
    this->p9_act = _p9_act;
    this->p10_act = _p10_act;
}

FSMachine::~FSMachine()
{
    if(this->curGait.valid())
        this->curGait.wait();
}


//time based FSM

void FSMachine::PushSen(int *curMea)
{
    int hipDiff = this->mvf.DataFilt(curMea[LANKPOS] + curMea[RANKPOS] - this->LHipMean - this->RHipMean);
    if (this->curIdx < POS_BUF_SIZE)
    {
        // std::cout << this->curIdx << std::endl;
        this->LHipBuf[this->curIdx] = curMea[LHIPPOS];
        this->RHipBuf[this->curIdx] = curMea[RHIPPOS];
        this->LKneBuf[this->curIdx] = curMea[LKNEPOS];
        this->RKneBuf[this->curIdx] = curMea[RKNEPOS];
        this->LAnkBuf[this->curIdx] = curMea[LANKPOS];
        this->RAnkBuf[this->curIdx] = curMea[RANKPOS];
        this->curIdx++;

         //determine should we start the new gait
        
        this->time = curMea[TIME];
        if ((this->preHipDiff > 0) && (hipDiff < 0)) //need more criterion, need threshold
        {
            //if we found the legs switch (left leg is at the front again), 
            //if the previous gait hasn't end, we will just wait
            //reset is not the option since it is common to have previous gait longer then the current one. 
            //yet, we set it to default time if the buffer overflow
            std::scoped_lock<std::mutex> curLock(this->gaitLock);
            if(!this->isWalkFlag){
                this->isWalkFlag = true;
                this->curGait = std::async(&FSMachine::_OneGait,this);
            }
            else{
                std::cout<<"gait has not ended\n";
            }
        
        }
        
    }
    else{
        //if it overflow, we need to reset
        this->Reset();
        std::cout << "FSM overflow\n";
    }
    this->preHipDiff = hipDiff;   
    

    
}
void FSMachine::Reset()
{
    
    this->swIdx = 0;
    this->curIdx = 0;
    
}

void FSMachine::GetP1()
{
    // int *swPoint = std::min_element(this->LAnkBuf + this->swIdx, this->LAnkBuf + this->curIdx - 1);
    // this->p1_idx = swPoint - this->LAnkBuf;
    this->p1_idx = 60;
}
void FSMachine::GetP2()
{
    // this->p2_idx = this->p1_idx + 80;
    this->p2_idx = 60;
}
void FSMachine::GetP3()
{
    // int *swPoint = std::min_element(this->LKneBuf + this->swIdx, this->LKneBuf + this->curIdx - 1);
    // this->p3_idx = swPoint - this->LKneBuf;
    this->p3_idx = 60;
}
void FSMachine::GetP4()
{
    // int *swPoint = std::max_element(this->RKneBuf + this->swIdx, this->RKneBuf + this->curIdx - 1);
    // this->p4_idx = swPoint - this->RKneBuf;
    this->p4_idx = 0;
}
void FSMachine::GetP5()
{
    // int *swPoint = std::min_element(this->RAnkBuf, this->RAnkBuf + this->swIdx);
    // this->p5_idx = swPoint - this->RAnkBuf;
    this->p5_idx = 60;
}
void FSMachine::GetP6()
{
    // int *swPoint = std::max_element(this->RAnkBuf, this->RAnkBuf + this->swIdx);
    // this->p6_idx = swPoint - this->RAnkBuf;
    this->p6_idx = 60;
}
void FSMachine::GetP7()
{
    // this->p7_idx = this->p6_idx + 80;
    this->p7_idx = 60;
}
void FSMachine::GetP8()
{
    // int *swPoint = std::max_element(this->RKneBuf, this->RKneBuf + this->swIdx);
    // this->p8_idx = swPoint - this->RKneBuf;
    this->p8_idx = 60;
}
void FSMachine::GetP9()
{
    // int *swPoint = std::min_element(this->LKneBuf, this->LKneBuf + this->swIdx);
    // this->p9_idx = swPoint - this->LKneBuf;
    this->p9_idx = 60;
}
void FSMachine::GetP10()
{
    // int *swPoint = std::max_element(this->LKneBuf + this->swIdx, this->LKneBuf + this->curIdx - 1);
    // this->p10_idx = swPoint - this->LKneBuf;
    this->p10_idx = 60;
}
void FSMachine::CalPhaseTime()
{
   
    this->GetP1();
    this->GetP2();
    this->GetP3();
    this->GetP4();
    this->GetP5();
    this->GetP6();
    this->GetP7();
    this->GetP8();
    this->GetP9();
    this->GetP10();
    
}

void FSMachine::SetInitPos(int curLHip, int curRHip)
{
    this->RHipMean = curRHip;
    this->LHipMean = curLHip;
    std::cout << "set init pos, LHip:" << curLHip << ", RHip:" << curRHip << std::endl;
    this->InitPosRec->PushData(this->initPosSetTime++,std::vector<int>(curLHip,curRHip));
}
void FSMachine::_OnePhase(std::function<void()> *actFun, int *time_idx){
    this->gaitTime.tv_nsec += *time_idx*USEC*SAMPTIME;
    (*actFun)();
    Common::tsnorm(&this->gaitTime);
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &this->gaitTime, NULL);
}
void FSMachine::_OneGait(){
    this->CalPhaseTime(); //this may have problem since it will probably take some time
    clock_gettime(CLOCK_MONOTONIC, &this->gaitTime);
    this->_OnePhase(&this->p5_act, &this->p5_idx);
    this->_OnePhase(&this->p6_act, &this->p6_idx);
    this->_OnePhase(&this->p7_act, &this->p7_idx);
    this->_OnePhase(&this->p8_act, &this->p8_idx);
    this->_OnePhase(&this->p9_act, &this->p9_idx);
    this->_OnePhase(&this->p10_act, &this->p10_idx);
    this->_OnePhase(&this->p1_act, &this->p1_idx);
    this->_OnePhase(&this->p2_act, &this->p2_idx);
    this->_OnePhase(&this->p3_act, &this->p3_idx);
    this->_OnePhase(&this->p4_act, &this->p4_idx); //we don't need to have operating time since it should end when new gait starts
    std::cout << "gait end\n";
    {
        std::scoped_lock<std::mutex> curLock(this->gaitLock);
        this->isWalkFlag = false;
    }
    
}
