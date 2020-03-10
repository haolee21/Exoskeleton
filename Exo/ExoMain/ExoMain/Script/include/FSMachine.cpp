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
    

    this->curIdx = 0;
    this->gaitStart = false;
    //init buffer
    this->LHipBuf = this->LHipBuf_array;
    this->LHipBuf_pre = this->LHipBuf_pre_array;
    this->RHipBuf = this->RHipBuf_array;
    this->RHipBuf_pre = this->RHipBuf_pre_array;
    this->LKneBuf = this->LKneBuf_array;
    this->LKneBuf_pre = this->LKneBuf_pre_array;
    this->RKneBuf = this->RKneBuf_array;
    this->RKneBuf_pre = this->RKneBuf_pre_array;
    this->LAnkBuf = this->LAnkBuf_array;
    this->LAnkBuf_pre = this->LAnkBuf_pre_array;
    this->RAnkBuf = this->RAnkBuf_array;
    this->RAnkBuf_pre = this->RAnkBuf_pre_array;

    this->FSMRec.reset(new Recorder<int>("FSM", filePath, "time,phase1,phase2,phase3,phase4,phase5,phase6,phase7,phase8,phase9,phase10,period,sw"));
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

    this->curState = 9;
}

FSMachine::~FSMachine()
{
    if(this->curGait.valid())
        this->curGait.wait();
}


//time based FSM

void FSMachine::PushSen(int *curMea)
{
    int hipDiff = this->mvf.DataFilt(curMea[LHIPPOS] + curMea[RHIPPOS] - this->LHipMean - this->RHipMean);
    this->time = curMea[TIME];
    if (this->curIdx < POS_BUF_SIZE){
        this->LHipBuf[this->curIdx] = curMea[LHIPPOS];
        this->RHipBuf[this->curIdx] = curMea[RHIPPOS];
        this->LKneBuf[this->curIdx] = curMea[LKNEPOS];
        this->RKneBuf[this->curIdx] = curMea[RKNEPOS];
        this->LAnkBuf[this->curIdx] = curMea[LANKPOS];
        this->RAnkBuf[this->curIdx] = curMea[RANKPOS];
        this->curIdx++;
        //std::cout << "prediff: " << this->preHipDiff << ", curDiff: " << hipDiff << std::endl;
        if (((this->preHipDiff > 0) && (hipDiff <= 0)))        {
            if(!this->gaitStart){
                this->Reset();//this order matters, this is why I cannot have reset in the outside scope
                // std::cout << "gait started\n";
                this->gaitStart = true;
                this->findSw = false;
                
            }
            else if(this->findSw){
                this->findSw = false;
                std::scoped_lock<std::mutex> curLock(this->gaitLock);
                if(!this->isWalkFlag){
                    this->isWalkFlag = true;
                    
                    
                    
                    this->curGait = std::async(&FSMachine::_OneGait, this, this->curIdx,this->swIdx);
                    this->Reset();
                    this->gaitStart = true;
                }
                else{
                    // std::cout<<"gait has not ended\n";
                    this->Reset();
                    this->gaitStart = true;
                }
            }
            else{
                
                // std::cout << "cannot find switch point\n";
                this->Reset();
            }
        }

        
        else if ((this->preHipDiff<0) && (hipDiff>=0)){//when another leg is front (we should have 2 hipDiff=0 point in one gait)
            if(this->gaitStart){
                if(!this->findSw){
                    this->findSw = true;
                    this->swIdx = this->curIdx;
                    // std::cout << "switching point\n";
                }
                else{
                    // std::cout << "task did not activate\n";
                    this->Reset();
                }    
            }
            
            else{
                
                // std::cout << "cannot find init point\n";
                this->Reset();
            }
        }
        
    }
    else{
        //if it overflow, we need to reset
        this->Reset();
        // std::cout << "FSM overflow\n";
    }
    
    this->preHipDiff = hipDiff;   
    

    
}
void FSMachine::Reset()
{
    // std::cout << "reset FSM\n";
    this->gaitStart = false;
    this->findSw = false;
    this->curIdx = 0;
}

void FSMachine::GetP1()
{
    int *swPoint = std::min_element(this->LAnkBuf_pre + this->p3_idx-(this->period>>2), this->LAnkBuf_pre+this->p3_idx);
    // std::cout << "p1 search range: " << this->p3_idx - (this->period >> 2) << ',' << this->p3_idx << std::endl;
    this->p1_idx = swPoint - this->LAnkBuf_pre;
    if(this->p1_idx<0)
        this->idx_less_0 = true;
}
void FSMachine::GetP2()
{
    
    this->p2_idx = (this->p1_idx+this->p3_idx)>>1;
    if(this->p2_idx<0)
        this->idx_less_0 = true;
}
void FSMachine::GetP3()
{
    int *minPoint = std::min_element(this->LKneBuf_pre+this->swIdx_pre-(this->period>>2),this->LKneBuf_pre+this->swIdx_pre);
    // std::cout << "p3 search range: " << this->swIdx_pre - (this->period >> 2) << ',' << this->swIdx_pre << std::endl;

    this->p3_idx = minPoint - this->LKneBuf_pre;
    if(this->p3_idx<0){
        this->idx_less_0 = true;
    }
}
void FSMachine::GetP4()
{
    int *minPoint = std::max_element(this->RKneBuf_pre+this->swIdx_pre-(this->period>>2),this->RKneBuf_pre+this->swIdx_pre);
    this->p4_idx = minPoint - this->RKneBuf_pre;
    if(this->p4_idx<0){
        this->idx_less_0 = true;
    }
}
void FSMachine::GetP5()
{
    int *swPoint = std::min_element(this->RKneBuf_pre+this->p4_idx, this->RKneBuf_pre + this->p4_idx+(this->period>>2));
    this->p5_idx = swPoint - this->RKneBuf_pre;
    if(this->p5_idx<0){
        this->idx_less_0 = true;
    }
}
void FSMachine::GetP6()
{
    int *swPoint = std::max_element(this->RAnkBuf_pre+this->p8_idx-(this->period>>2), this->RAnkBuf_pre + this->p8_idx);
    this->p6_idx = swPoint - this->RAnkBuf_pre;
    if(this->p6_idx<0){
        this->idx_less_0 = true;
    }
}
void FSMachine::GetP7()
{
    
    this->p7_idx = (this->p6_idx+this->p8_idx)>>1;
    if(this->p7_idx<0){
        this->idx_less_0 = true;
    }
}
void FSMachine::GetP8()
{
    int *maxPoint = std::max_element(this->RKneBuf_pre+this->period-(this->period>>2), this->RKneBuf_pre +this->period);
    this->p8_idx = maxPoint - this->RKneBuf_pre;
    if(this->p8_idx<0){
        this->idx_less_0 = true;
    }
}
void FSMachine::GetP9()
{
    int *minPoint = std::min_element(this->LKneBuf_pre+this->period-(this->period>>2), this->LKneBuf_pre +this->period);
    this->p9_idx = minPoint - this->LKneBuf_pre;
    if(this->p9_idx<0){
        this->idx_less_0 = true;
    }
    
}
void FSMachine::GetP10()
{
    int *swPoint = std::max_element(this->LKneBuf_pre, this->LKneBuf_pre+(this->period>>2));
    this->p10_idx = swPoint - this->LKneBuf_pre;
    if(this->p10_idx<0){
        this->idx_less_0 = true;
    }
}
void FSMachine::CalPhaseTime()
{
    this->idx_less_0 = false;
    this->_swapBuf(&this->LHipBuf, &this->LHipBuf_pre);
    this->_swapBuf(&this->RHipBuf, &this->RHipBuf_pre);
    this->_swapBuf(&this->LKneBuf, &this->LKneBuf_pre);
    this->_swapBuf(&this->RKneBuf, &this->RKneBuf_pre);
    this->_swapBuf(&this->LAnkBuf, &this->LAnkBuf_pre);
    this->_swapBuf(&this->RAnkBuf, &this->RAnkBuf_pre);
    this->GetP3();
    this->GetP4();
    this->GetP8();
    this->GetP9();
    this->GetP1();
    this->GetP6();
    this->GetP2();
    this->GetP7();
    this->GetP5();
    this->GetP10();

    this->FSMRec->PushData(this->time, std::vector<int>{this->p1_idx, this->p2_idx, this->p3_idx, this->p4_idx, this->p5_idx, this->p6_idx, this->p7_idx, this->p8_idx, this->p9_idx, this->p10_idx,this->period,this->swIdx});
    if(this->idx_less_0){
        std::cout << "false time\n";
        this->p5_idx = 1;
        this->p6_idx = 2;
        this->p7_idx = 3;
        this->p8_idx = 4;
        this->p9_idx = 5;
        this->p10_idx = 6;
        this->p1_idx = 7;
        this->p2_idx = 8;
        this->p3_idx = 9;
        this->p4_idx = 10;
    }
}

void FSMachine::SetInitPos(int curLHip, int curRHip)
{
    this->RHipMean = curRHip;
    this->LHipMean = curLHip;
    std::cout << "set init pos, LHip:" << curLHip << ", RHip:" << curRHip << std::endl;
    this->InitPosRec->PushData(this->initPosSetTime++,std::vector<int>{curLHip,curRHip});
}
void FSMachine::_OnePhase(std::function<void()> *actFun, int *time_idx){
    this->gaitTime.tv_nsec += *time_idx*USEC*SAMPTIME;
    Common::tsnorm(&this->gaitTime);
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &this->gaitTime, NULL);
    (*actFun)();
    
}
void FSMachine::_OneGait(int period,int swIdx){
    
    //two criteria need to be satisfied, we need to make sure the switch point does not happen at the beginning or very end of each gait
    //criterion 1: still have 1/4 period before swIdx1
    //criterion 2: still have 1/4 period after swIdx2
    std::cout << "current period: " << period <<", current sw: "<<swIdx << std::endl;
    
    if ((swIdx - (period >> 2) > 0) && (swIdx + (period >> 2) < period))
    {
        this->swIdx_pre = swIdx;
        this->period = period;
        this->CalPhaseTime(); //this may have problem since it will probably take some time
        clock_gettime(CLOCK_MONOTONIC, &this->gaitTime);
        this->_OnePhase(&this->p10_act, &this->p10_idx);
        this->_OnePhase(&this->p1_act, &this->p1_idx);
        this->_OnePhase(&this->p2_act, &this->p2_idx);
        this->_OnePhase(&this->p3_act, &this->p3_idx);
        this->_OnePhase(&this->p4_act, &this->p4_idx); //we don't need to have operating time since it should end when new gait starts
        this->_OnePhase(&this->p5_act, &this->p5_idx);
        this->_OnePhase(&this->p6_act, &this->p6_idx);
        this->_OnePhase(&this->p7_act, &this->p7_idx);
        this->_OnePhase(&this->p8_act, &this->p8_idx);
        this->_OnePhase(&this->p9_act, &this->p9_idx);
        
        
        std::cout << "gait end\n";
        

    }
    else{
        // std::cout<<"not a complete gait\n";
        //std::cout << "swIdx=" << this->swIdx << ",period: "<<this->period<<std::endl;
    }
    {
        std::scoped_lock<std::mutex> curLock(this->gaitLock);
        this->isWalkFlag = false;
    }
}

void FSMachine::_swapBuf(int **buf, int **pre_buf){
    int *curBuf = *buf;
    *buf = *pre_buf;
    *pre_buf = curBuf;
    return;
}

void FSMachine::ManualAdv(){

    switch (this->curState++)
    {
    case 1:
        this->p2_act();
        break;
    case 2:
        this->p3_act();
        break;
    case 3:
        this->p4_act();
        break;
    case 4:
        this->p5_act();
        break;
    case 5:
        this->p6_act();
        break;
    case 6:
        this->p7_act();
        break;
    case 7:
        this->p8_act();
        break;
    case 8:
        this->p9_act();
        break;
    case 9:
        this->p10_act();
        break;
    case 10:
        this->p1_act();
        break;

    default:
        break;
    }
    if(this->curState==11){
        this->curState = 1;
    }
}