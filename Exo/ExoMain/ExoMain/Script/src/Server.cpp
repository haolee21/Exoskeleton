#include "Server.hpp"

using namespace boost::asio;
Server::Server(recAct _task_handler)
{
    this->task_handler=_task_handler;
    this->ios.reset(new io_service);
    this->acceptor.reset(
        new ip::tcp::acceptor(*(this->ios),
                                           ip::tcp::endpoint(ip::tcp::v4(), 1234)));
    this->socket = new ip::tcp::socket(*(this->ios));
    std::cout << "[Exo] Looking for clinets\n";
    this->acceptor->accept(*(this->socket));
    std::cout << "[Exo] Connect to clinet\n";

    this->wait_msg = true;
    this->th_read = new std::thread(&Server::readServer, this);
}

Server::~Server()
{
    this->wait_msg = false;
}
void Server::readServer()
{
    while(this->wait_msg){
        streambuf buf;
        read_until(*(this->socket), buf, '\n');
        streambuf::const_buffers_type bufs = buf.data();
        std::string str(buffers_begin(bufs), buffers_begin(bufs) + bufs.size());
        this->task_handler(str);
    }
}
void Server::WriteServer(const std::string &message)
{
    const std::string data = message + '\n';
    write(*(this->socket), boost::asio::buffer(data));
}
