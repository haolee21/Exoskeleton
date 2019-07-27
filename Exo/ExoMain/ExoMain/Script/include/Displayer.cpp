#include "Displayer.hpp"
Displayer::Displayer()
{
    boost::asio::io_service ios;
    boost::asio::ip::tcp::socket sock(ios);
    this->s = &sock;
    tcp::endpoint endPoint(ip::address::from_string("192.168.1.142"),12345);
    std::cout<<"[client] Searching\n";
    s->connect(endPoint);
    std::cout<<"[client] Connect"<<std::endl;
}

Displayer::~Displayer()
{
}
void Displayer::send(const char *msg,const int dataLen){
    boost::asio::write(*this->s,boost::asio::buffer(msg,dataLen));
}