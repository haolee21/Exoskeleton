#ifndef DISPLAYER_HPP
#define DISPLAYER_HPP
#include<boost/asio.hpp>
#include<iostream>
using namespace boost::asio;
using ip::tcp;



class Displayer
{ 
private:
    tcp::socket *s;
    void send(tcp::socket *s,const char *msg,const int dataLen);
public:
    Displayer(/* args */);
    ~Displayer();
};

Displayer::Displayer(/* args */)
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
void Displayer::send(tcp::socket *s,const char *msg,const int dataLen){
    boost::asio::write(*s,boost::asio::buffer(msg,dataLen));
}
#endif //DISPLAYER_HPP