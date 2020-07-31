#ifndef CONNECTOR_H
#define CONNECTOR_H
#include <boost/asio.hpp>
#include <iostream>
class Connector
{
private:
    boost::asio::io_service *ios;
    boost::asio::ip::tcp::socket *socket;
    
    bool portSet;
    

public:
    Connector(/* args */);
    ~Connector();
    void Connect();
    void Disconnect();
    void Send(const std::string &message);
    void Read();
    void SetIp(std::string ip, int port);
    std::string ip;
    int port;
    bool isConnect; //boost asio does not have function to check whetheer it is connected
};
#endif
