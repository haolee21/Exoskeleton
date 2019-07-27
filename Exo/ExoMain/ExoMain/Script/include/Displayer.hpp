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
    
public:
    Displayer();
    void send(const char *msg,const int dataLen);
    ~Displayer();
};

#endif //DISPLAYER_HPP