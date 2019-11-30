#include "PWM.h"

#include <vector>

// #define MY_PRIORITY (49)             /* we use 49 as the PRREMPT_RT use 50 \
//                                         as the priority of kernel tasklets \
//                                         and interrupt handler by default */
// #define POOLSIZE (200 * 1024 * 1024) // 200MB
// #define MAX_SAFE_STACK (100 * 1024)  /* The maximum stack size which is \
//                                       guranteed safe to access without  \
//                                       faulting */

#define NSEC_PER_SEC (1000000000) // The number of nsecs per sec.

#define NSEC 1
#define USEC (1000 * NSEC)
#define MSEC (1000 * USEC)
#define SEC (1000 * MSEC)

typedef std::chrono::duration<unsigned long, std::micro> microsecs_t;

PWMGen::PWMGen(std::string valveName, std::string filePath, int pinId, int _sampT, int _pwmIdx, std::chrono::system_clock::time_point _origin)
{
	this->origin = _origin;
	this->pwmIdx = _pwmIdx;
	this->sampT = _sampT;
	// this->onTime =0;
	// this->SetDuty(0,0);
	// this->pwmRec = new Recorder<int>(valveName,filePath,"time,"+valveName);
	this->pwmRec.reset(new Recorder<int>(valveName, filePath, "time," + valveName));
	this->pwmOnOffRec.reset(new Recorder<int>(valveName + "OnOff", filePath, "time," + valveName));

	this->DutyLock = new std::mutex;

	this->gpioPin.reset(new Pin(pinId));

	//original pid control parameters are all 0
	this->pid.reset(new PIDCon(0, 0, 0, 0));
}
void PWMGen::SetDuty(int onDuty, int curTime)
{
	this->duty.num = onDuty;
	{
		std::lock_guard<std::mutex> lock(*this->DutyLock);
		if(!this->swDuty){
			this->CalTime(onDuty);
			this->swDuty = false;
		}
		
	}

	std::vector<int> curDuty;
	curDuty.push_back(onDuty);
	this->pwmRec->PushData((unsigned long)curTime, curDuty);
}
void PWMGen::CalTime(int duty)
{
	this->onTime = this->sampT * duty / 100;
}
void PWMGen::Start()
{
	this->on = true;
	this->pwmTh.reset(new std::thread(&PWMGen::Mainloop, this));
}
void PWMGen::Stop()
{
	this->on = false;
	this->pwmTh->join();
}
void PWMGen::Mainloop()
{

	//for accurate timer
	struct timespec t;
	//struct sched_param param;
	long int interval = this->sampT * USEC;

	clock_gettime(CLOCK_MONOTONIC, &t);
	// t.tv_nsec += 0 * MSEC;
	// // this->tsnorm(&t);
	// Common::tsnorm(&t);
	//

	while (this->on)
	{

		int curOnTime;
		{
			std::lock_guard<std::mutex> lock(*this->DutyLock);
			curOnTime = this->onTime;
			this->swDuty = false;
		}
		std::chrono::system_clock::time_point curT = std::chrono::system_clock::now();
		microsecs_t start_time(std::chrono::duration_cast<microsecs_t>(curT - this->origin));
		std::vector<int> pwmDataOn;
		pwmDataOn.push_back(curOnTime);
		this->pwmOnOffRec->PushData(start_time.count(), pwmDataOn);
		this->gpioPin->On();

		//t.tv_nsec += curOnTime * USEC;
		usleep(curOnTime); //at first I tried clock_nanosleep, but maybe due to accuracy issue, this method is more stable
		//clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t, NULL);
		this->gpioPin->Off();

		// timer
		// calculate next shot
		//t.tv_nsec += (interval-curOnTime*USEC);
		t.tv_nsec += interval;
		// this->tsnorm(&t);
		Common::tsnorm(&t);
		clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t, NULL);
	}
}
int PWMGen::GetIdx()
{
	return this->pwmIdx;
}

PWMGen::~PWMGen()
{
	this->on = false;
	this->gpioPin->Off();

	delete this->DutyLock;
}

void PWMGen::PushMea(int curTime, float curMea)
{
	if (curMea > -100.0)
	{
		int curDuty;
		curDuty = this->pid->GetDuty(curMea);
		

		if ((this->duty.num - curDuty > 5) || (this->duty.num - curDuty < -5))
		{
			this->SetDuty(curDuty, curTime);
		}
	}
	else{
		this->SetDuty(0, curTime);
	}
}
void PWMGen::SetPID_const(float _kp, float _kd, float _ki, int preMea)
{
	this->pid->UpDatePID(_kp, _ki, _kd);
	this->pid->SetInit(preMea);
}