#include "Encoder.hpp"
Encoder::Encoder(int pinId)
{
    this->_initCE(pinId);
    this->fd = open("/dev/spidev0.0", O_RDWR);
    unsigned int speed = 1000000;
    ioctl (this->fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
}

Encoder::~Encoder()
{
}
void Encoder::_initCE(int pinId){
    switch (pinId)
    {
    case 0:
        this->pinC = false;
        this->pinB = false;
        this->pinA = false; 
        break;
    case 1:
        this->pinC = false;
        this->pinB = false;
        this->pinA = true;
        break;
    case 2:
        this->pinC = false;
        this->pinB = true;
        this->pinA = false;
        break;
    case 3:
        this->pinC = false;
        this->pinB = true;
        this->pinA = true;
        break;
    case 4:
        this->pinC = true;
        this->pinB = false;
        this->pinA = false;
        break;
    case 5:
        this->pinC = true;
        this->pinB = false;
        this->pinA = true;
        break;
    case 6:
        this->pinC = true;
        this->pinB = true;
        this->pinA = false;
        break;
    case 7:
        this->pinC = true;
        this->pinB = true;
        this->pinA = true;
        break;
    default:
        break;
    }
   
}
void Encoder::_setCE(){
    if(this->pinA)
        this->CEA_pin.On();
    else
        this->CEA_pin.Off();
    if(this->pinB)
        this->CEB_pin.On();
    else
        this->CEB_pin.Off();
    if(this->pinC)
        this->CEC_pin.On();
    else
        this->CEC_pin.Off();
}
void Encoder::SetZero(){
    bool still_setting = true;
    char rxData;
    rxData=this->_spiTxRx(0x70);

    while(rxData!=0x80){
        rxData = this->_spiTxRx(0x00);
        usleep(20);
    }
}
char Encoder::_spiTxRx(char txData){
    char rxData;
    struct spi_ioc_transfer spi;
    memset (&spi, 0, sizeof (spi));
    spi.tx_buf = (unsigned long)&txData;
    spi.rx_buf = (unsigned long)&rxData;
    spi.len    = 1;
    ioctl (this->fd,SPI_IOC_MESSAGE(1), &spi);
    return rxData;
}
int Encoder::ReadPos(){
    //preset the CE pins
    this->_setCE();
    //send read_pos 
    char rxData = this->_spiTxRx(0x10);
    while(rxData!=0x10){
        usleep(20);
        rxData = this->_spiTxRx(0x00);
    }
    usleep(20);
    char MSB = this->_spiTxRx(0x00);
    char LSB = this->_spiTxRx(0x00);
    return (int)(8 << MSB) + (int)LSB;
}