#include "Encoder.hpp"
#include <chrono>
#include <sstream>

using namespace std;
Encoder::Encoder(int pinId,int spi_num)
{
    int ret = 0;

    this->_initCE(pinId);
    stringstream ss;
    ss<<spi_num;
    string spi_port = "/dev/spidev"+ss.str()+".0";
    this->fd = open(spi_port.c_str(), O_RDWR);
    unsigned int speed = 1000000;


    auto mode = SPI_MODE_0;
    ret = ioctl(this->fd, SPI_IOC_WR_MODE, &mode);
    if (ret == -1)
        cout << "cannot set spi write mode\n";

    ret = ioctl(this->fd, SPI_IOC_RD_MODE, &mode);
    if (ret == -1)
        cout << "cannot set spi read mode\n";

    uint8_t bits = 8;
    ret = ioctl(this->fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
    if (ret < 0)
        cout << "cannot set bits per work\n";

    ret = ioctl(this->fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
    if (ret < 0)
        cout << "cannot set bits per work\n";

    
    ret = ioctl(this->fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
    if (ret < 0)
        cout << "cannot set write speed\n";
    ret = ioctl(this->fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
    if (ret < 0)
        cout << "cannot set read speed\n";

    int lsb_first = 0;
    ret = ioctl(this->fd, SPI_IOC_RD_LSB_FIRST, &lsb_first);
    if (ret < 0)
        cout << "cannot set read MSB first\n";
    ret = ioctl(this->fd, SPI_IOC_WR_LSB_FIRST, &lsb_first);
    if (ret < 0)
        cout << "cannot set write MSB first\n";
}

Encoder::~Encoder()
{
}
void Encoder::_initCE(int pinId)
{
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
void Encoder::_setCE()
{
    if (this->pinA)
        this->CEA_pin.On();
    else
        this->CEA_pin.Off();
    if (this->pinB)
        this->CEB_pin.On();
    else
        this->CEB_pin.Off();
    if (this->pinC)
        this->CEC_pin.On();
    else
        this->CEC_pin.Off();
}
void Encoder::SetZero()
{
    this->_setCE();

    this->txBuf[0] = 0x70;
    this->_spiTxRx(1);

    while (rxBuf[0] != 0x80)
    {
        usleep(20);
        this->txBuf[0]=0x00;
        this->_spiTxRx(1);
    }
    cout<<"done set zero\n";
}
char Encoder::_spiTxRx(unsigned int len)
{
    // normal spi protocal can take more than 1 byte, however, for our encoder, sending two bytes is not applicable, sending 1 byte at a time
    struct spi_ioc_transfer spi[len];
    memset(&spi, 0, sizeof(spi));
    spi->tx_buf = (unsigned long)this->txBuf;
    spi->rx_buf = (unsigned long)this->rxBuf;
    spi->len = len;
    return ioctl(this->fd, SPI_IOC_MESSAGE(1), &spi);
    
}
int Encoder::ReadPos()
{
    //preset the CE pins
    // auto start = chrono::high_resolution_clock::now();
    this->_setCE();
    
    //send read_pos
    this->txBuf[0] = 0x10;
    this->_spiTxRx(1);
    // this->txBuf[0]=0x00;
    while (this->rxBuf[0] != 0x10)
    {
        usleep(20);
        this->_spiTxRx(1);
        
    }
    // usleep(20);
    
    this->txBuf[0] = 0x00;
    this->_spiTxRx(1);
    this->MSB = this->rxBuf[0];
    this->_spiTxRx(1);
    this->LSB = this->rxBuf[0];


    // auto elapsed = chrono::high_resolution_clock::now() - start;
    // long long dur_time = chrono::duration_cast<chrono::microseconds>(elapsed).count();
    // cout << "time: " << dur_time << endl;
    return (int)(this->MSB << 8) + (int)this->LSB;
   
}