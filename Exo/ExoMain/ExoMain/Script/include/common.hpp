#ifndef COMMON_HPP
#define COMMON_HPP
#define NUMSEN 16
#define SAMPTIME 1000 //this is in micro second

#define DATALEN (NUMSEN*2+1)
#define SIZEOFBUFFER  (DATALEN*10)




#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sched.h>
#include <sys/mman.h>
#include <string.h>
#include <pthread.h>

#include <errno.h>

#include <iostream>
#include <fcntl.h>
#include <sched.h>
#include <sys/io.h>
#include <string.h>
#include <signal.h>

#include <malloc.h>

#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>

#include <sys/ioctl.h>
#define POOLSIZE (200 * 1024 * 1024) // 200MB
#define MAX_SAFE_STACK (100 * 1024)  /* The maximum stack size which is \
                                      guranteed safe to access without  \
                                      faulting */


int initialize_memory_allocation(void);
void stack_prefault(void);

#define NSEC_PER_SEC (1000000000) // The number of nsecs per sec.

#define NSEC 1
#define USEC (1000 * NSEC)
#define MSEC (1000 * USEC)
#define SEC (1000 * MSEC)
// //===================================For Pin.h =======================================================================
// #define PAGE_SIZE (4*1024)
// #define BLOCK_SIZE (4*1024)

// int  mem_fd;
// void *gpio_map;

// // I/O access
// volatile unsigned *gpio;


// // GPIO setup macros. Always use INP_GPIO(x) before using OUT_GPIO(x) or SET_GPIO_ALT(x,y)
// #define INP_GPIO(g) *(gpio+((g)/10)) &= ~(7<<(((g)%10)*3))
// #define OUT_GPIO(g) *(gpio+((g)/10)) |=  (1<<(((g)%10)*3))
// #define SET_GPIO_ALT(g,a) *(gpio+(((g)/10))) |= (((a)<=3?(a)+4:(a)==4?3:2)<<(((g)%10)*3))

// #define GPIO_SET *(gpio+7)  // sets   bits which are 1 ignores bits which are 0
// #define GPIO_CLR *(gpio+10) // clears bits which are 1 ignores bits which are 0

// #define GET_GPIO(g) (*(gpio+13)&(1<<g)) // 0 if LOW, (1<<g) if HIGH

// #define GPIO_PULL *(gpio+37) // Pull up/pull down
// #define GPIO_PULLCLK0 *(gpio+38) // Pull up/pull down clock
// //=======================================================================================================================

#define TIME 0
#define LHIPPOS 1
#define LKNEPOS 2
#define LANKPOS 3
#define RHIPPOS 4
#define RKNEPOS 5
#define RANKPOS 6

#define SYNCREAD 7

#define TANKPRE 9
#define LKNEPRE 10
#define LANKPRE 11
#define RKNEPRE 12
#define RANKPRE 13

//Some setting constant
#define RELTIME 10 //time that the valve will open to release pressure



#define VALNUM 7 //this cannot work with test reacting
#define PWMNUM 4

class Common
{
private:
    /* data */
public:
    Common(/* args */);
    ~Common();
    static void tsnorm(struct timespec *ts);
};



#endif