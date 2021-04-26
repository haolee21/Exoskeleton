#ifndef PIN_H
#define PIN_H
// reference from https://elinux.org/RPi_GPIO_Code_Samples
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define VALUE_MAX 30
class Pin
{  
public:
    enum IO_TYPE
    {
        Output,
        Input
    };
    Pin(int _pinId, Pin::IO_TYPE io_type);
    ~Pin();
    void On();
    void Off();
    int Read();

private:
    int GPIOExport(int pin);
    int GPIOUnexport(int pin);
    int GPIODirection(int pin, int dir);
    int GPIORead(int pin);
    int GPIOWrite(int value);
    int pinId;
    IO_TYPE iotype;

    //parameters need to declare first to make write function faster
    int fd_wirte;
    bool writingFlag=false; //since we will keep it open during operation, we need a flag to track it
};

#endif