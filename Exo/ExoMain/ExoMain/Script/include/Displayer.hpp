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
    std::unique_ptr<boost::asio::io_service> ios;
    std::unique_ptr<tcp::socket> s;
    
public:
    Displayer();
  
    void send(char*,unsigned int );
    ~Displayer();
};

#endif //DISPLAYER_HPP