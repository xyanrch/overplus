#include "Server.h"
#include <boost/asio/io_context.hpp>
#include <cstdlib>
#include <memory>

Server::Server(const std::string& address, const std::string& port)
    : context_pool(2)
    , io_context(context_pool.get_io_context())
   // , signals(io_context)
    , acceptor_(io_context)
    , ssl_ctx(boost::asio::ssl::context::tlsv13)
{
    add_signals();
    ip::tcp::resolver resover(io_context);
    local_endpoint = *resover.resolve(address, port).begin();

  // start_accept();
}

void Server::start_accept()
{
    acceptor_.open(local_endpoint.protocol());
    acceptor_.set_option(ip::tcp::acceptor::reuse_address(true));
    acceptor_.bind(local_endpoint);
    acceptor_.listen();
    do_accept();
}
void Server::do_accept()
{
    std::shared_ptr<Session> new_session = std::make_shared<Session>(context_pool.get_io_context(), ssl_ctx);
    acceptor_.async_accept(new_session->socket(), [this, new_session](const boost::system::error_code& ec) {
        if (!acceptor_.is_open()) {
            return;
        }
        if (ec == boost::asio::error::operation_aborted) {
            NOTICE_LOG << "got cancel signal, stop calling myself";
            return;
        }
        if (!ec) {
            boost::system::error_code error;
            auto ep = new_session->socket().remote_endpoint(error);
            NOTICE_LOG << "accept incoming connection :" << ep.address().to_string();
            new_session->start();
        } else {
            NOTICE_LOG << "accept incoming connection fail:" << ec.message();
        }
        do_accept();
    });
}
void Server::run()
{
    NOTICE_LOG << "Server start..." << std::endl;
    context_pool.run();
}
void Server::stop()
{
    NOTICE_LOG << "Server stopped..." << std::endl;
    context_pool.stop();
}
void Server::stop_accept()
{
    acceptor_.cancel();
    acceptor_.close();
}
void Server::add_signals()
{
   /* signals.add(SIGINT);
    signals.add(SIGTERM);
#ifdef SIGQUIT
    signals.add(SIGQUIT);
#endif
    signals.async_wait([this](const boost::system::error_code& ec, int sig) {
        acceptor_.close();

        NOTICE_LOG << "Server stopped..." << std::endl;
        exit(1);
    });*/
}
