#include "Pin.hpp"
Pin::Pin(int _pinID)
{
    //wiringPiSetup ();
    this->pinId = _pinID;
    wiringPiSetup();
    pinMode(_pinID,OUTPUT);

}

Pin::~Pin()
{
}
void Pin::On(){
    std::cout<<this->pinId<<" is on\n";
    digitalWrite(this->pinId,HIGH);
}
void Pin::Off(){
    std::cout<<this->pinId<<" is off\n";
    digitalWrite(this->pinId,LOW);
}