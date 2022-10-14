#include "Server.h"
#include <boost/asio/io_context.hpp>
#include <cstdlib>
#include <memory>

Server::Server(const std::string& address, const std::string& port)
    : context_pool(5)
    , io_context(context_pool.get_io_context())
    , signals(io_context)
    , acceptor_(io_context)
    , ssl_ctx(boost::asio::ssl::context::tlsv13)
{
    add_signals();
    ip::tcp::resolver resover(io_context);
    ip::tcp::endpoint endpoint = *resover.resolve(address, port).begin();


    acceptor_.open(endpoint.protocol());
    acceptor_.set_option(ip::tcp::acceptor::reuse_address(true));
    acceptor_.bind(endpoint);
    acceptor_.listen();
    do_accept();
}
void Server::do_accept()
{
    std::shared_ptr<Session> new_session = std::make_shared<Session>(context_pool.get_io_context(),ssl_ctx);
    acceptor_.async_accept(new_session->socket(), [this, new_session](const boost::system::error_code& ec) {
        if (!acceptor_.is_open()) {
            return;
        }
        if (ec == boost::asio::error::operation_aborted) {
            NOTICE_LOG << "got cancel signal, stop calling myself";
            return;
        }
        if (!ec) {
            NOTICE_LOG << "accept incoming connection :" << ec.message() << std::endl;
            new_session->start();
        } else {
            NOTICE_LOG << "accept incoming connection fail:" << ec.message() << std::endl;
        }
        do_accept();
    });
}
void Server::run()
{
    NOTICE_LOG << "Server start..." << std::endl;
    context_pool.run();
}
void Server::add_signals()
{
    signals.add(SIGINT);
    signals.add(SIGTERM);
#ifdef SIGQUIT
    signals.add(SIGQUIT);
#endif
    signals.async_wait([this](const boost::system::error_code& ec, int sig) {
        acceptor_.close();

        NOTICE_LOG << "Server stopped..." << std::endl;
        exit(1);
    });
}
