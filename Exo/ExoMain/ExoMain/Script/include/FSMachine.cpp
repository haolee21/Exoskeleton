#include "FSMachine.hpp"
FSMachine::FSMachine(std::string filePath)
{
    this->LHipBuf.reset(new int[POS_BUF_SIZE]);
    this->RHipBuf.reset(new int[POS_BUF_SIZE]);
    this->LKneBuf.reset(new int[POS_BUF_SIZE]);
    this->RKneBuf.reset(new int[POS_BUF_SIZE]);
    this->LAnkBuf.reset(new int[POS_BUF_SIZE]);
    this->RAnkBuf.reset(new int[POS_BUF_SIZE]);

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
    this->timeReady = false;
    this->halfGait = false;
    this->gaitStart = false;
    this->FSMRec.reset(new Recorder<int>("FSM", filePath, "time,phase1,phase2,phase3,phase4,phase5,phase6,phase7,phase8,phase9,phase10"));
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
char FSMachine::CalState(int *curMea, char curState)
{

    switch (curState)
    {
    case Phase1:
        if (curMea[LKNEPOS] > LKNE_MIN_POS)
        {
            return Phase1;
        }
        else
        {
            std::cout << "Phase2\n";
            return Phase2;
        }
        break;
    case Phase2:
        if (curMea[LHIPPOS] > LHIP_MIN_POS)
        {
            return Phase2;
        }
        else
        {
            std::cout << "Phase3\n";
            return Phase3;
        }
        break;
    case Phase3:
        if (curMea[LANKPOS] > LANK_PUSH_POS)
        {
            return Phase3;
        }
        else
        {
            std::cout << "Phase4\n";
            return Phase4;
        }
        break;
    case Phase4:
        if (curMea[RKNEPOS] < RKNE_REC_POS)
        {
            return Phase4;
        }
        else
        {
            std::cout << "Phase5\n";
            return Phase5;
        }
        break;
    case Phase5:
        if (curMea[RKNEPOS] < RKNE_ENDREC_POS)
        {
            return Phase5;
        }
        else
        {
            std::cout << "Phase6\n";
            return Phase6;
        }
        break;
    case Phase6:
        if (curMea[RHIPPOS] < RHIP_PUSH_POS)
        {
            return Phase6;
        }
        else
        {
            std::cout << "Phase7\n";
            return Phase7;
        }
        break;
    case Phase7:
        if (curMea[RANKPOS] < RANK_ENDPUSH_POS)
        {
            return Phase7;
        }
        else
        {
            std::cout << "Phase8\n";
            return Phase8;
        }
        break;
    case Phase8:
        if (curMea[LKNEPOS] < LKNE_REC_POS)
        {
            return Phase8;
        }
        else
        {
            std::cout << "Phase1\n";
            return Phase1;
        }
        break;
    default:
        return curState;
    }
}

//time based FSM

void FSMachine::PushSen(int *curMea)
{
    int hipDiff = this->mvf.DataFilt(curMea[LHIPPOS] + curMea[RHIPPOS] - this->LHipMean - this->RHipMean);

    if ((this->preHipDiff > 0) && (hipDiff < 0))
    {
        // before the first step, we have to wait for the starting point
        // but after the first step, whenever we switch the leg, we just need to re init the FSM
        if (!this->gaitStart)
        {
            this->gaitStart = true;
        }
        else
        {
            this->CalPhaseTime();
            this->Reset();
        }
    }
    if (this->gaitStart)
    {
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
        }
        else
        {
            //std::cout << "buffer is full\n";
            // std::lock_guard<std::mutex> lock(this->lock);
            //this->timeReady = true;
        }
        if (!halfGait)
        {
            if ((this->preHipDiff < 0) && (hipDiff > 0))
            {
                this->halfGait = true;
                this->swIdx = this->curIdx - 1;
            }
        }
    }

    this->preHipDiff = hipDiff;
}
void FSMachine::Reset()
{
    this->swIdx = 0;
    this->curIdx = 0;
    // this->p1_idx = 0;
    // this->p2_idx = 0;
    // this->p3_idx = 0;
    // this->p4_idx = 0;
    // this->p5_idx = 0;
    // this->p6_idx = 0;
    // this->p7_idx = 0;
    // this->p8_idx = 0;
    // this->p9_idx = 0;
    // this->p10_idx = 0;
}
void FSMachine::GetPhaseTime(int curTime, long &p1_t, long &p2_t, long &p3_t, long &p4_t, long &p5_t,
                             long &p6_t, long &p7_t, long &p8_t, long &p9_t, long &p10_t)
{
    std::cout << "calculate phase time\n";
    {
        std::lock_guard<std::mutex> lock(this->lock);
        if (this->timeReady)
        {
            // p1_t = (unsigned long)this->p1_idx * MSEC;
            // p2_t = (unsigned long)this->p2_idx * MSEC;
            // p3_t = (unsigned long)this->p3_idx * MSEC;
            // p4_t = (unsigned long)this->p4_idx * MSEC;
            // p5_t = (unsigned long)this->p5_idx * MSEC;
            // p6_t = (unsigned long)this->p6_idx * MSEC;
            // p7_t = (unsigned long)this->p7_idx * MSEC;
            // p8_t = (unsigned long)this->p8_idx * MSEC;
            // p9_t = (unsigned long)this->p9_idx * MSEC;
            // p10_t = (unsigned long)this->p10_idx * MSEC;
            p1_t = (long)100*MSEC;
            p2_t = (long)100*MSEC;
            p3_t = (long)100*MSEC;
            p4_t = (long)100*MSEC;
            p5_t = (long)100*MSEC;
            p6_t = (long)100*MSEC;
            p7_t = (long)100*MSEC;
            p8_t = (long)100*MSEC;
            p9_t = (long)100*MSEC;
            p10_t = (long)100*MSEC;
            this->timeReady = false;
        }
        
    }
    p1_t = (long)500*MSEC;
    p2_t = (long)500*MSEC;
    p3_t = (long)500*MSEC;
    p4_t = (long)500*MSEC;
    p5_t = (long)500*MSEC;
    p6_t = (long)500*MSEC;
    p7_t = (long)500*MSEC;
    p8_t = (long)500*MSEC;
    p9_t = (long)500*MSEC;
    p10_t = (long)500*MSEC;
    std::vector<int> data = std::vector<int>{this->p1_idx, this->p2_idx, this->p3_idx, this->p4_idx, this->p5_idx, this->p6_idx, this->p7_idx, this->p8_idx, this->p9_idx, this->p10_idx};
    this->FSMRec->PushData((unsigned long)curTime, data);
    std::cout << p1_idx << ',' << p2_idx << ',' << p3_idx << ',' << p4_idx << ',' << p5_idx << ',' << p6_idx << ',' << p7_idx << ',' << p8_idx << ',' << p9_idx << ',' << p10_idx << std::endl;
}
void FSMachine::GetP1()
{
    int *swPoint = std::min_element(this->LAnkBuf.get() + this->swIdx, this->LAnkBuf.get() + this->curIdx - 1);
    this->p1_idx = swPoint - this->LAnkBuf.get();
}
void FSMachine::GetP2()
{
    this->p2_idx = this->p1_idx + 80;
}
void FSMachine::GetP3()
{
    int *swPoint = std::min_element(this->LKneBuf.get() + this->swIdx, this->LKneBuf.get() + this->curIdx - 1);
    this->p3_idx = swPoint - this->LKneBuf.get();
}
void FSMachine::GetP4()
{
    int *swPoint = std::max_element(this->RKneBuf.get() + this->swIdx, this->RKneBuf.get() + this->curIdx - 1);
    this->p4_idx = swPoint - this->RKneBuf.get();
}
void FSMachine::GetP5()
{
    int *swPoint = std::min_element(this->RAnkBuf.get(), this->RAnkBuf.get() + this->swIdx);
    this->p5_idx = swPoint - this->RAnkBuf.get();
}
void FSMachine::GetP6()
{
    int *swPoint = std::max_element(this->RAnkBuf.get(), this->RAnkBuf.get() + this->swIdx);
    this->p6_idx = swPoint - this->RAnkBuf.get();
}
void FSMachine::GetP7()
{
    this->p7_idx = this->p6_idx + 80;
}
void FSMachine::GetP8()
{
    int *swPoint = std::max_element(this->RKneBuf.get(), this->RKneBuf.get() + this->swIdx);
    this->p8_idx = swPoint - this->RKneBuf.get();
}
void FSMachine::GetP9()
{
    int *swPoint = std::min_element(this->LKneBuf.get(), this->LKneBuf.get() + this->swIdx);
    this->p9_idx = swPoint - this->LKneBuf.get();
}
void FSMachine::GetP10()
{
    int *swPoint = std::max_element(this->LKneBuf.get() + this->swIdx, this->LKneBuf.get() + this->curIdx - 1);
    this->p10_idx = swPoint - this->LKneBuf.get();
}
void FSMachine::CalPhaseTime()
{
    std::lock_guard<std::mutex> lock(this->lock);
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
    this->timeReady = true;
}
bool FSMachine::IsReady()
{
    bool curCond;
    {
        std::lock_guard<std::mutex> lock(this->lock);
        curCond = this->timeReady;
    }

    return curCond;
}
void FSMachine::SetInitPos(int curLHip, int curRHip)
{
    this->RHipMean = curRHip;
    this->LHipMean = curLHip;
    std::cout << "set init pos, LHip:" << curLHip << ", RHip:" << curRHip << std::endl;
}