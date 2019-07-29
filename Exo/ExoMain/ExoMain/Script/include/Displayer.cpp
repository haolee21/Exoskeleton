#include "Displayer.hpp"
Displayer::Displayer()
{
    boost::asio::io_service ios;
    boost::asio::ip::tcp::socket sock(ios);
    // this->s = new boost::asio::ip::tcp::socket(ios);
    this->s.reset(new boost::asio::ip::tcp::socket(ios));
    tcp::endpoint endPoint(ip::address::from_string("192.168.1.142"),12345);
    std::cout<<"[client] Searching\n";
    s->connect(endPoint);
    std::cout<<"[client] Connect"<<std::endl;
}

Displayer::~Displayer()
{
    // delete this->s;
}
void Displayer::send(std::string msg){
    this->s->send(boost::asio::buffer(msg));
    
}