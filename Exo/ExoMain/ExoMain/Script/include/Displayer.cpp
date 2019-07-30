#include "Displayer.hpp"
Displayer::Displayer()
{
    boost::asio::io_service ios;
    boost::asio::ip::tcp::socket sock(ios);
   
    this->s.reset(new boost::asio::ip::tcp::socket(ios));
    tcp::endpoint endPoint(ip::address::from_string("192.168.1.142"),12345);
    std::cout<<"[client] Searching\n";
    s->connect(endPoint);
    std::cout<<"[client] Connect"<<std::endl;
}

Displayer::~Displayer()
{
    std::cout<<"try to shutdown\n";
    this->s->shutdown(boost::asio::ip::tcp::socket::shutdown_both);
    this->s->close();
    //I am not sure why this cannot shutdown the socket, at least in controller the client will maintain exist
    //unless I explicitly delete it 
}

void Displayer::send(char *buff,unsigned int dataLen){
    this->s->send(boost::asio::buffer(buff,dataLen));
}