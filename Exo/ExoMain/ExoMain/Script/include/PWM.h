#ifndef PWM_H
#define PWM_H
#include <mutex>
#include<wiringPi.h>
#include<thread>
#include<Recorder.hpp>
#include <string>
#include <chrono>
#include<ctime> //this timer
#include<memory>
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
	Duty duty;


private:
	std::chrono::system_clock::time_point origin;
	int pinId;
	int pwmIdx;
	bool on = false;
	void Mainloop();
	std::shared_ptr<std::thread> pwmTh;
	void CalTime(int duty);
	
	//calculate on time
	int onTime;
	

	// Recorder<int> *pwmRec;
	std::shared_ptr<Recorder<int>> pwmRec;
	std::shared_ptr<Recorder<int>> pwmOnOffRec;
	//timer 
	void tsnorm(struct timespec *ts);
	int sampT;
	long int ms_cnt = 0;
};

#endif //PWM_H