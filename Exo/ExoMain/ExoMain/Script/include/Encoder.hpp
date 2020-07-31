// Reference from http://robotics.hobbizine.com/raspiduino.html
#ifndef ENCODER_HPP
#define ENCODER_HPP
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <fcntl.h>
#include <iostream>
#include <cstring>
#include <Pin.h>

//index for CE of encoders
#define CEA 7
#define CEB 1
#define CEC 0

class Encoder
{
private:
    void _initCE(int pinId); //set the chip select pin
    void _setCE();
    bool pinA;
    bool pinB;
    bool pinC;
    Pin CEA_pin = Pin(CEA);
    Pin CEB_pin = Pin(CEB);
    Pin CEC_pin = Pin(CEC);
    int fd;
    char _spiTxRx(char txData);
    

public:
    Encoder(int pinId);
    ~Encoder();
    void SetZero();
    int ReadPos();
};


#endif //ENCODER