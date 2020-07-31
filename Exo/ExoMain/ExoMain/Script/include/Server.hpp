#include <boost/asio.hpp>
#include <iostream>
#include <thread>
#include <memory>
#include <functional>
typedef std::function<void(const std::string &)> recAct;
class Server
{
private:
    std::unique_ptr<boost::asio::io_service> ios;
    std::unique_ptr<boost::asio::ip::tcp::acceptor> acceptor;
    boost::asio::ip::tcp::socket *socket;
    std::thread *th_client_search;
    void search_client();
    bool flag_search_client = false;
    recAct task_handler;
    void readServer();
    bool wait_msg;
    std::thread *th_read;

public:
    Server(recAct _task_handler); //callback function for handle the received msg
    ~Server();
    
    void WriteServer(const std::string &msg);
};

