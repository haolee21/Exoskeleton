#include "ValveCon.hpp"
ValveCon::ValveCon(std::string filePath,std::chrono::system_clock::time_point _t_origin)
{
    this->t_origin = _t_origin;
    this->fd = open("/dev/i2c-1", O_RDWR);
    if (fd < 0) {
        std::cout << "Error opening file: " << strerror(errno) << '\n';
    }   
    if (ioctl(fd, I2C_SLAVE, I2C_ADDR) < 0) {
        std::cout << "ioctl error: " << strerror(errno) << "\n";
        
    }
    //initialize recorder, and set all valves to be zero
    
    this->valRec.reset(new Recorder<bool,7>("Val", filePath, "time," + this->val_col));
    this->pwmRec.reset(new Recorder<int,8>("PWM", filePath, "time," + this->pwm_col));
    this->setVal(LKNE_VAL,ValveCon::ValSwitch::Off);
    this->setVal(RKNE_VAL,ValveCon::ValSwitch::Off);
    this->setVal(LANK_VAL,ValveCon::ValSwitch::Off);
    this->setVal(RANK_VAL,ValveCon::ValSwitch::Off);
    this->setVal(LBAL_VAL,ValveCon::ValSwitch::Off);
    this->setVal(RBAL_VAL,ValveCon::ValSwitch::Off);
    this->setVal(PREL_VAL,ValveCon::ValSwitch::Off);

    this->setDuty(LHIP_PWM,0);
    this->setDuty(RHIP_PWM,0);
    this->setDuty(LKNE_PWM,0);
    this->setDuty(RKNE_PWM,0);
    this->setDuty(LANK_PWM,0);
    this->setDuty(RANK_PWM,0);
    this->setDuty(DRIV_PWM,0);
    this->setDuty(PREC_PWM,0);

}

ValveCon::~ValveCon()
{
}

void ValveCon::setDuty(int pwm_idx,int duty){
    auto elapsed = std::chrono::high_resolution_clock::now() - this->t_origin;
    long long curTime = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
    if(duty>99)
        duty = 99;
    this->curCmd[pwm_idx] = duty;
    this->pwmCond[pwm_idx]=duty;

    this->pwmRec->PushData(curTime,this->pwmCond);
    this->_send_cmd();
}
void ValveCon::setVal(int val_idx,bool cond){
    auto elapsed = std::chrono::high_resolution_clock::now() - this->t_origin;
    long long curTime = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
    this->curCmd[val_idx] = cond;
    this->valCond[val_idx]=cond;
    this->valRec->PushData(curTime,this->valCond);
    this->_send_cmd();
}
void ValveCon::_send_cmd(){
    if(write(this->fd,&this->curCmd,NUM_PWM+NUM_VAL) != NUM_PWM+NUM_VAL){
        std::cout << "i2c error\n";
    }
}
void ValveCon::pwm_all_off(){
    for (int i = 0; i < NUM_PWM;i++){
        this->curCmd[i] = 99;
    }
    this->_send_cmd();
}
void ValveCon::val_all_off(){
    for (int i = 0; i < NUM_VAL;i++){
        this->curCmd[NUM_PWM+i] = ValSwitch::Off;
    }
    this->_send_cmd();
}
void ValveCon::preRel(){
    std::cout << "Pressure Release\n";
    this->pwm_all_off();
    this->val_all_off();
    this->curCmd[PREL_VAL] = ValSwitch::On;
    this->_send_cmd();
    sleep(RELTIME / 2);

    this->curCmd[LKNE_VAL] = ValSwitch::Off;
    this->curCmd[RKNE_VAL] = ValSwitch::Off;
    this->curCmd[LANK_VAL] = ValSwitch::Off;
    this->curCmd[RANK_VAL] = ValSwitch::Off;
    this->curCmd[LANK_PWM] = 99;
    this->curCmd[RANK_PWM] = 99;
    this->_send_cmd();
    sleep(RELTIME / 2);

    this->pwm_all_off();
    this->val_all_off();
}