#include "PWM.h"

PWMGen::PWMGen(int pinId)
{
	this->DutyLock = new std::mutex();
	this->pinId = pinId;
	pinMode(this->pinId, OUTPUT);
}
void PWMGen::SetDuty(int onDuty) {
	this->DutyLock->lock();
	this->onTime = onDuty * this->timeFactor;
	this->offTime = this->totalTime - this->onTime;
	this->DutyLock->unlock();
}
std::thread *PWMGen::Start() {
	std::thread *mainThread = new std::thread(&PWMGen::Mainloop, this);
	return mainThread;

}
void PWMGen::Mainloop() {
	while (this->on) {
		
		this->DutyLock->lock();
		int curOnTime = this->onTime;
		int curOffTime = this->offTime;
		this->DutyLock->unlock();
		digitalWrite(this->pinId, HIGH);
		delayMicroseconds(curOnTime);
		digitalWrite(this->pinId, LOW);
		delayMicroseconds(curOffTime);
	}
}
std::mutex *PWMGen::GetDutyLock() {
	return this->DutyLock;
}
PWMGen::~PWMGen()
{
}