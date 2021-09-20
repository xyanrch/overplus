#include "Server.h"
#include <boost/asio/io_context.hpp>
#include <cstdlib>
#include <memory>

Server::Server(const std::string& address, const std::string& port)
    : context_pool(5)
    , io_context(context_pool.get_io_context())
    , signals(io_context)
    , acceptor_(io_context)
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
    new_session.reset(new Session(context_pool.get_io_context()));
    acceptor_.async_accept(new_session->socket(), [this](auto ec) {
        if (!acceptor_.is_open()) {
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
    io_context.run();
}
void Server::add_signals()
{
    signals.add(SIGINT);
    signals.add(SIGTERM);
#ifdef SIGQUIT
    signals.add(SIGQUIT);
#endif
    signals.async_wait([this](auto ec, auto sig) {
        acceptor_.close();

        NOTICE_LOG << "Server stopped..." << std::endl;
        exit(1);
    });
}
