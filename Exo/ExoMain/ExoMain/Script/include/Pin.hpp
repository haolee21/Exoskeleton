#ifndef Pin_HPP
#define Pin_HPP
#include <wiringPi.h>
#include <iostream>
class Pin
{
private:
    int pinId;
public:
    Pin(int pinID);
    ~Pin();
    void On();
    void Off();
    
};

#endif