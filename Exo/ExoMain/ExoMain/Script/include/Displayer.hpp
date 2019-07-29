#ifndef DISPLAYER_HPP
#define DISPLAYER_HPP
#include<boost/asio.hpp>
#include<iostream>
#include<string>
#include <memory>
using namespace boost::asio;
using ip::tcp;



class Displayer
{ 
private:
    //tcp::socket *s;
    std::unique_ptr<tcp::socket> s;
    
public:
    Displayer();
    void send(std::string msg);
    ~Displayer();
};

#endif //DISPLAYER_HPP