#ifndef COMMON_HPP
#define COMMON_HPP
#define NUMSEN 16
#define SAMPTIME 1000 //this is in micro second




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
static void exo_error(int at);
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
#endif