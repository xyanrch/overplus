#pragma once
#include "ServerSession.h"
#include <Shared/ConfigManage.h>
#include <Shared/IoContextPool.h>
#include <Shared/Log.h>
#include <boost/asio.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/ssl/context.hpp>
#include <memory>
using namespace boost::asio;

class SslServer : private boost::noncopyable {
public:
    SslServer();
    void run();

private:
    void add_signals();
    void do_accept();
    void load_server_certificate(boost::asio::ssl::context& ctx);

private:
    IoContextPool context_pool;
    // ios_deque io_contexts_;
    boost::asio::io_context& io_context;

    boost::asio::signal_set signals;
    boost::asio::ip::tcp::acceptor acceptor_;
    boost::asio::ssl::context ssl_context_;
    std::shared_ptr<ServerSession> new_connection_;
};
