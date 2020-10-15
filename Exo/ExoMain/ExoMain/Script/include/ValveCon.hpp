// Valve controller works on Teensy 4.0
#ifndef VALVECON_HPP
#define VALVECON_HPP

#include <iostream>
#include <linux/i2c-dev.h>
#include <fcntl.h>    /* For O_RDWR */
#include <unistd.h>   /* For open(), */
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include "Recorder.hpp"
#include "common.hpp"
#define I2C_ADDR 0x002D //this address has to sync with the address on teensy board
#define NUM_PWM 8
#define NUM_VAL 7

#define LHIP_PWM 0
#define RHIP_PWM 1
#define LKNE_PWM 2
#define RKNE_PWM 3
#define LANK_PWM 4
#define RANK_PWM 5
#define DRIV_PWM 6
#define PREC_PWM 7

#define LKNE_VAL (NUM_PWM+0)
#define RKNE_VAL (NUM_PWM+1)
#define LANK_VAL (NUM_PWM+2)
#define RANK_VAL (NUM_PWM+3)
#define LBAL_VAL (NUM_PWM+4)
#define RBAL_VAL (NUM_PWM+5)
#define PREL_VAL (NUM_PWM+6)
class ValveCon
{
private:
    int fd;
    std::string filePath;
    Recorder<bool,7> *valRec;
    Recorder<int,8> *pwmRec;
    char curCmd[NUM_PWM + NUM_VAL]; //save the cmd, so if we want to change one of the valve, we can change arrordingly 
    void _send_cmd();

    std::string pwm_col = "LHipPwm,RHipPWM,LKnePWM,RKnePWM,LAnkPWM,RAnkPWM,DrivePWM,PRecPWM";
    std::string val_col = "LKneVal,RKneVal,LAnkVal,RAnkVal,LBalVal,RBalVal,PRelVal";

public:
    ValveCon(std::string filePath);
    ~ValveCon();
    
    void setDuty(int pwm,int duty);
    void setVal(int val,bool cond);
    
    void pwm_all_off();
    
    void val_all_off();
    void preRel();
};

#endif VALVECON_HPP