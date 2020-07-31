#include "Connector.h"
using namespace boost::asio;
Connector::Connector(/* args */)
{
    this->ip = "192.168.0.15";
    this->port = 1234;
    this->ios = new io_service;
    this->socket = new ip::tcp::socket(*this->ios);
    this->portSet = true;
    this->isConnect = false;
}
Connector::~Connector()
{
    this->Disconnect();
}
void Connector::Connect()
{
    if (!this->isConnect)
    {
        if (this->portSet)
        {
            std::cout << "[PC] Searching " << this->ip << "\n";
            ip::tcp::endpoint ep(ip::address::from_string(this->ip), this->port);
            this->socket->connect(ep);
            this->isConnect = true;
            std::cout << "[PC] Connect\n";
        }
        else
        {
            std::cout << "[PC] you have not define port\n";
        }
    }
}
void Connector::Disconnect()
{
    if (this->isConnect)
    {
        this->socket->shutdown(ip::tcp::socket::shutdown_both);
        this->isConnect = false;
    }
}
void Connector::Send(const std::string &message)
{
    const std::string data = message + '\n';
    write(*this->socket, buffer(data));
    std::cout << "send data " << data;
}
void Connector::Read()
{
    streambuf buf;
    read_until(*this->socket, buf, '\n');
}
void Connector::SetIp(std::string _ip,int _port){
    this->ip = _ip;
    this->port = _port;
    this->portSet = true;
}