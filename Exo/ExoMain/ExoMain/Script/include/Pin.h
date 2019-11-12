#ifndef PIN_H
#define PIN_H

#define BCM2708_PERI_BASE        0x3F000000
#define GPIO_BASE                (BCM2708_PERI_BASE + 0x200000) /* GPIO controller */

#include "common.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#define PAGE_SIZE (4*1024)
#define BLOCK_SIZE (4*1024)


class Pin
{
private:
    void setup_io();
    int pinId;
    /* data */
    int test =0;
public:
    Pin(int pinId);
    ~Pin();
    void On();
    void Off();
    
};

#endif