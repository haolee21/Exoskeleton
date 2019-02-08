#ifndef PWM_H
#define PWM_H
#include <mutex>
#include<wiringPi.h>
#include<thread>

class PWMGen
{
public:
	PWMGen(int);
	~PWMGen();
	void SetDuty(int);
	std::thread *Start();
	std::mutex *GetDutyLock();
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
	std::mutex *DutyLock;
};

#endif // !PWM_H