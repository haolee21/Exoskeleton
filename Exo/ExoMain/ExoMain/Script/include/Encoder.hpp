// Reference from http://robotics.hobbizine.com/raspiduino.html
#ifndef ENCODER_HPP
#define ENCODER_HPP
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <fcntl.h>
#include <iostream>
#include <cstring>
#include "Pin.hpp"

//index for CE of encoders
#define CEA 5
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
    
    Pin CEA_pin = Pin(CEA,Pin::IO_TYPE::Output);
    Pin CEB_pin = Pin(CEB,Pin::IO_TYPE::Output);
    Pin CEC_pin = Pin(CEC,Pin::IO_TYPE::Output);
    int fd;
    char _spiTxRx(unsigned int len);

    char rxBuf[5];
    char txBuf[5];
    char MSB;
    char LSB;

public:
    Encoder(int pinId,int spi_num);
    ~Encoder();
    void SetZero();
    int ReadPos();
};


#endif //ENCODER