#include "PWM.h"

#include <vector>

#define MY_PRIORITY (49)             /* we use 49 as the PRREMPT_RT use 50 \
                                        as the priority of kernel tasklets \
                                        and interrupt handler by default */
#define POOLSIZE (200 * 1024 * 1024) // 200MB
#define MAX_SAFE_STACK (100 * 1024)  /* The maximum stack size which is \
                                      guranteed safe to access without  \
                                      faulting */

#define NSEC_PER_SEC (1000000000) // The number of nsecs per sec.

#define NSEC 1
#define USEC (1000 * NSEC)
#define MSEC (1000 * USEC)
#define SEC (1000 * MSEC)


//timer
void PWMGen::tsnorm(struct timespec *ts)
{
    while (ts->tv_nsec >= NSEC_PER_SEC)
    {
        ts->tv_nsec -= NSEC_PER_SEC;
        ts->tv_sec++;
    }
}
//
PWMGen::PWMGen(std::string valveName, std::string filePath,int pinId,int _sampT,int _pwmIdx)
{
	this->pwmIdx = _pwmIdx;
	this->sampT = _sampT;
	// this->onTime =0;
	// this->SetDuty(0,0);
	this->pwmRec = new Recorder<int>(valveName,filePath,"time,"+valveName);
	this->DutyLock = new std::mutex;
	this->pinId = pinId;
	wiringPiSetup(); 
	pinMode(this->pinId, OUTPUT);
}
void PWMGen::SetDuty(int onDuty,int curTime) {
	this->duty.num=onDuty;
	{
		std::lock_guard<std::mutex> lock(*this->DutyLock);
		this->CalTime(onDuty);
	}
	
	std::vector<int> curDuty;
	curDuty.push_back(onDuty);
	this->pwmRec->PushData((unsigned long)curTime,curDuty);

}
void PWMGen::CalTime(int duty){
	this->onTime=this->sampT*duty/100;
}
std::thread *PWMGen::Start() {
	this->on = true;
	std::thread *mainThread = new std::thread(&PWMGen::Mainloop, this);
	return mainThread;

}
void PWMGen::Stop(){
	this->on = false;
}
void PWMGen::Mainloop() {
	//for accurate timer
	struct timespec t;
    //struct sched_param param;
    long int interval = this->sampT*USEC;
   
	clock_gettime(CLOCK_MONOTONIC, &t);
	t.tv_nsec += 0 * MSEC;
    this->tsnorm(&t);
	//

	while (this->on) {
		//timer
		clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t, NULL);
		//
		int curOnTime;
		{
			std::lock_guard<std::mutex> lock(*this->DutyLock);
			curOnTime = this->onTime;
		}
		
		digitalWrite(this->pinId, HIGH);
		delayMicroseconds(curOnTime);
		digitalWrite(this->pinId, LOW);
		

		// timer
		// calculate next shot
        t.tv_nsec += interval;
        this->tsnorm(&t);
		//

	}
}
int PWMGen::GetIdx(){
	return this->pwmIdx;
}

PWMGen::~PWMGen()
{
	this->on = false;
	digitalWrite(this->pinId, LOW);
	delete this->pwmRec;
	delete this->DutyLock;
}