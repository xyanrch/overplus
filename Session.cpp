
#include "Session.h"
#include <boost/asio/io_context.hpp>
Session::Session(boost::asio::io_context& context)
    : socket_(context)
{
}

void Session::start()
{
    do_read();
}
void Session::do_read()
{
}

 boost::asio::ip::tcp::socket& Session::socket()
 {
     return socket_;
 }