#ifndef PWM_H
#define PWM_H
#include <mutex>
#include<wiringPi.h>
#include<thread>
#include<Recorder.hpp>
#include <string>
#include <chrono>
#include<ctime> //this timer
union Duty
{
    int num;
    char byte[4];
};
class PWMGen
{
	
public:
	PWMGen(std::string valveName,std::string filePath, int pinId,int _sampTMicro,int pwmIdx);
	~PWMGen();
	void SetDuty(int onDuty,int curTime);
	std::thread *Start();
	std::mutex* DutyLock;
	void Stop();
	int GetIdx();
private:
	int pinId;
	int pwmIdx;
	bool on = false;
	void Mainloop();

	void CalTime(int duty);
	
	//calculate on time
	int onTime;
	

	Recorder<int> *pwmRec;
	//timer 
	void tsnorm(struct timespec *ts);
	int sampT;
	long int ms_cnt = 0;
};

#endif //PWM_H