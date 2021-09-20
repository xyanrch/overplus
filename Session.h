
#pragma once
#include <boost/asio.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/core/noncopyable.hpp>
#include <memory>
using  boost::asio::ip::tcp;
class Session : public std::enable_shared_from_this<Session>
    , private boost::noncopyable {
public:
    Session(boost::asio::io_context&context);

    void start();
    void do_read();
    boost::asio::ip::tcp::socket&socket();

private:
    //socket for connection
    tcp::socket socket_;
};