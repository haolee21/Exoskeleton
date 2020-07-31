#ifndef VALVE_H
#define VALVE_H
#include <Arduino.h>
class Valve
{
private:
    int pin_id;

public:
    Valve(int);
    ~Valve();
    void on();
    void off();
};

Valve::Valve(int _pin_id)
{
    this->pin_id = _pin_id;
}

Valve::~Valve()
{
}
void Valve::on(){
    digitalWrite(this->pin_id, true);
}
void Valve::off(){
    digitalWrite(this->pin_id, false);
}
#endif
