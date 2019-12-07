#ifndef PWM_H
#define PWM_H
#include <mutex>
#include "Pin.h"
#include<thread>
#include<Recorder.hpp>
#include <string>
#include <chrono>
#include<ctime> //this timer
#include<memory>
#include <unistd.h>
#include "PIDCon.h"
#include<pthread.h>
#include "common.hpp"
#include <time.h>
union Duty
{
    int num;
    char byte[4];
};

class PWMGen
{
	
public:
	PWMGen(std::string valveName,std::string filePath, int pinId,int _sampTMicro,int pwmIdx,std::chrono::system_clock::time_point origin);
	~PWMGen();
	void SetDuty(int onDuty,int curTime);
	void Start();
	std::mutex* DutyLock;
	void Stop();
	int GetIdx();
	Duty duty; //the reason why it is public is because I use it in the controller to update the display, it should be rewrote
	void SetPID_const(float kp, float kd, float ki,int preIn);
	void PushMea(int curTime,float curMea);

private:
	std::chrono::system_clock::time_point origin;
	std::shared_ptr<Pin> gpioPin;
	int pwmIdx;
	bool on = false;
	void Mainloop();
	std::shared_ptr<std::thread> pwmTh;
	void CalTime(int duty);
	std::shared_ptr<PIDCon> pid;
	
	//calculate on time
	int onTime;
	
	//this is for experiment, for some reason function involves pid failed
	// I suspected it is due to rapidly setting duty in pwm
	// here I set a flag, if the duty changed, flag = true, only if it has been ran once it will be false
	bool swDuty = false;
	

	std::shared_ptr<Recorder<int>> pwmRec;
	std::shared_ptr<Recorder<int>> pwmOnOffRec;
	//timer 
	int sampT;
	long int ms_cnt = 0;

	
};

#endif //PWM_H