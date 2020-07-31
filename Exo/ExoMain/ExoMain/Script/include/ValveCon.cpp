#include "ValveCon.hpp"
ValveCon::ValveCon(std::string filePath)
{
    this->fd = open("/dev/i2c-1", O_RDWR);
    if (fd < 0) {
        std::cout << "Error opening file: " << strerror(errno) << '\n';
    }   
    if (ioctl(fd, I2C_SLAVE, I2C_ADDR) < 0) {
        std::cout << "ioctl error: " << strerror(errno) << "\n";
        
    }
    Recorder<bool> _valRec = Recorder<bool>("Val", filePath, "time," + this->val_col);
    this->valRec = &_valRec;
    Recorder<int> _pwmRec = Recorder<int>("PWM", filePath, "time," + this->pwm_col);
    this->pwmRec = &_pwmRec;
}

ValveCon::~ValveCon()
{
}

void ValveCon::setDuty(int pwm_idx,int duty){
    if(duty>99)
        duty = 99;
    this->curCmd[pwm_idx] = duty;
    this->_send_cmd();
}
void ValveCon::setVal(int val_idx,bool cond){
    this->curCmd[val_idx] = cond;
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
        this->curCmd[i] = false;
    }
    this->_send_cmd();
}
void ValveCon::preRel(){
    std::cout << "Pressure Release\n";
    this->pwm_all_off();
    this->val_all_off();
    this->curCmd[PREL_VAL] = true;
    this->_send_cmd();
    sleep(RELTIME / 2);

    this->curCmd[LKNE_VAL] = false;
    this->curCmd[RKNE_VAL] = false;
    this->curCmd[LANK_VAL] = false;
    this->curCmd[RANK_VAL] = false;
    this->curCmd[LANK_PWM] = 99;
    this->curCmd[RANK_PWM] = 99;
    this->_send_cmd();
    sleep(RELTIME / 2);

    this->pwm_all_off();
    this->val_all_off();
}