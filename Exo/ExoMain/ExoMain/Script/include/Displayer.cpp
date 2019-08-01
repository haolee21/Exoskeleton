#include "Displayer.hpp"
Displayer::Displayer()
{
    // boost::asio::io_service ios;
    // boost::asio::ip::tcp::socket sock(this->ios);
    
    this->s.reset(new boost::asio::ip::tcp::socket(this->ios));
    tcp::endpoint endPoint(ip::address::from_string("192.168.1.142"),12345);
    std::cout<<"[client] Searching\n";
    s->connect(endPoint);
    std::cout<<"[client] Connect"<<std::endl;
}

Displayer::~Displayer()
{
    
    this->s->shutdown(boost::asio::ip::tcp::socket::shutdown_both);
    this->s->close();
   
}

void Displayer::send(char *buff,unsigned int dataLen){
    this->s->send(boost::asio::buffer(buff,dataLen));
}