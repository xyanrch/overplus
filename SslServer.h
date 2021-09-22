#pragma once
#include "ConfigManage.h"
#include "IoContextPool.h"
#include "Log.h"
#include "ServerSession.h"
#include <boost/asio.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/ssl/context.hpp>
#include <memory>
using namespace boost::asio;

class SslServer : private boost::noncopyable {
public:
    SslServer(const std::string& config_path);
    void run();

private:
    void add_signals();
    void do_accept();

private:
    IoContextPool context_pool;
    // ios_deque io_contexts_;
    boost::asio::io_context& io_context;

    boost::asio::signal_set signals;
    boost::asio::ip::tcp::acceptor acceptor_;
    boost::asio::ssl::context ssl_context_;

private:
    ConfigManage config_manage;
};
