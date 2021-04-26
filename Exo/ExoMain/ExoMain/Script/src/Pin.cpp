#include "Pin.h"
#include <iostream>
#include <chrono>

#define LOW 0
#define HIGH 1
#define IN 0
#define OUT 1
using namespace std;
int Pin::GPIOExport(int pin)
{
#define BUFFER_MAX 3
    char buffer[BUFFER_MAX];
    ssize_t bytes_written;
    int fd;

    fd = open("/sys/class/gpio/export", O_WRONLY);
    if (-1 == fd)
    {
        fprintf(stderr, "Failed to open export for writing!\n");
        return (-1);
    }

    bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
    write(fd, buffer, bytes_written);
    close(fd);
    return (0);
}

int Pin::GPIOUnexport(int pin)
{
    char buffer[BUFFER_MAX];
    ssize_t bytes_written;
    int fd;

    fd = open("/sys/class/gpio/unexport", O_WRONLY);
    if (-1 == fd)
    {
        fprintf(stderr, "Failed to open unexport for writing!\n");
        return (-1);
    }

    bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
    write(fd, buffer, bytes_written);
    close(fd);
    return (0);
}

int Pin::GPIODirection(int pin, int dir)
{
    static const char s_directions_str[] = "in\0out";

#define DIRECTION_MAX 35
    char path[DIRECTION_MAX];
    int fd;

    snprintf(path, DIRECTION_MAX, "/sys/class/gpio/gpio%d/direction", pin);
    fd = open(path, O_WRONLY);
    if (-1 == fd)
    {
        fprintf(stderr, "Failed to open gpio direction for writing!\n");
        return (-1);
    }

    if (-1 == write(fd, &s_directions_str[IN == dir ? 0 : 3], IN == dir ? 2 : 3))
    {
        fprintf(stderr, "Failed to set direction!\n");
        return (-1);
    }

    close(fd);
    return (0);
}

int Pin::GPIORead(int pin)
{

    char path[VALUE_MAX];
    char value_str[3];
    int fd;

    snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
    fd = open(path, O_RDONLY);
    if (-1 == fd)
    {
        fprintf(stderr, "Failed to open gpio value for reading!\n");
        return (-1);
    }

    if (-1 == read(fd, value_str, 3))
    {
        fprintf(stderr, "Failed to read value!\n");
        return (-1);
    }

    close(fd);

    return (atoi(value_str));
}

int Pin::GPIOWrite(int value)
{

    static const char s_values_str[] = "01";



    if (1 != write(this->fd_wirte, &s_values_str[LOW == value ? 0 : 1], 1))
    {
        fprintf(stderr, "Failed to write value!\n");
        return (-1);
    }

    

    return (0);
}
void Pin::On()
{

    if (this->iotype == Pin::IO_TYPE::Output)
    {
        if (-1 == GPIOWrite( HIGH))
            cout << "Failed to set " << this->pinId << "on\n";
    }
    else
        cout << "This pin is input\n";
}
void Pin::Off()
{
    if (this->iotype == Pin::IO_TYPE::Output)
    {
        if (-1 == GPIOWrite(LOW))
            cout << "Failed to set " << this->pinId << "off\n";
    }
    else
    {
        cout << "This pin is output\n";
    }
}

int Pin::Read()
{
    int read_val = this->GPIORead(this->pinId);
    return read_val;
}

Pin::Pin(int _pinId, Pin::IO_TYPE _io_type)
{
    this->pinId = _pinId;
    this->iotype = _io_type;
    //enable gpio pins
    if (-1 == this->GPIOExport(this->pinId))
        cout << "Failed to enable gpio " << this->pinId << endl;
    // set gpio direction
    if (_io_type == Pin::IO_TYPE::Output)
    {
        if (-1 == this->GPIODirection(this->pinId, OUT))
            cout << "Failed to set gpio " << this->pinId << " to output\n";
    }
    else
    {
        if (-1 == this->GPIODirection(this->pinId, IN))
            cout << "Failed to set gpio " << this->pinId << " to input\n";
    }

    //declare parameter that GPIOWrite need earlier
    char path_write[VALUE_MAX];
    snprintf(path_write, VALUE_MAX, "/sys/class/gpio/gpio%d/value", _pinId);
    this->fd_wirte = open(path_write, O_WRONLY);
    if (-1 == this->fd_wirte)
    {
        fprintf(stderr, "Failed to open gpio value for writing!\n");
        
    }
}

Pin::~Pin()
{
    
    this->Off();
    if(this->writingFlag){
        close(this->fd_wirte);
    }
    if (-1 == this->GPIOUnexport(this->pinId))
        cout << "Failed to disable gpio " << this->pinId << endl;
}