#ifndef PWM_H
#define PWM_H
#include <mutex>
#include<wiringPi.h>
#include<thread>
#include<Recorder.hpp>
#include <string>
#include <chrono>
#include<ctime> //this timer
class PWMGen
{
	
public:
	PWMGen(std::string valveName,std::string filePath, int pinId,int _sampTMicro);
	~PWMGen();
	void SetDuty(int onDuty,int curTime);
	std::thread *Start();
	std::mutex* DutyLock;
	void Stop();
private:
	int pinId;
	
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