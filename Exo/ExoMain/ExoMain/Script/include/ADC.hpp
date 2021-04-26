#ifndef ADC_HPP
#define ADC_HPP




#include <cstring>
#include "Pin.hpp"
#include <memory>

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <iostream>
#define SEN1 1
#define SEN2 2
#define SEN3 3
#define SEN4 4
#define SEN5 5
#define SEN6 6
#define SEN7 7
#define SEN8 0
class ADC
{
private:
    int *data;
    void startConv();
    std::unique_ptr<Pin> conv;
    std::unique_ptr<Pin> reset;
    std::unique_ptr<Pin> eoc;
    void init_ADC();
    int fd;
    char _spiTxRx(unsigned int len);

    char ch_list[8];

    char rxBuf[100];
    char txBuf[100];

public:
    ADC(std::string device);
    ~ADC();
    void Read(int *data);
};


#endif //ADC_HPP