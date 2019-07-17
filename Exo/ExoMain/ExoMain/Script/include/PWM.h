#ifndef PWM_H
#define PWM_H
#include <mutex>
#include<wiringPi.h>
#include<thread>

class PWMGen
{
	
public:
	PWMGen(int pinId,int* duty,std::mutex* lock);
	~PWMGen();
	void SetDuty(int onDuty);
	std::thread *Start();
	std::mutex* DutyLock;
	void Stop();
private:
	int duty = 0;
	bool on = false;
	void Mainloop();

	int pinId;
	// time unit is micro second, 5000 us => 200 Hz
	int onTime = 0;
	int timeFactor = 50;
	int totalTime = 5000;
	int offTime = totalTime - onTime;

};

#endif //PWM_H