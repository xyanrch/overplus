#include "Service.h"
#include "Server/TlsSession.h"
#include "Shared/Log.h"
#include <boost/asio/io_context.hpp>
#include <boost/system/error_code.hpp>
#include <cstdlib>
#include <cstring>
//#include <filesystem>
#include <memory>
#include <string>

Service::Service()
        : context_pool(5), io_context(context_pool.get_io_context()), signals(io_context), acceptor_(io_context),
          ssl_context_(boost::asio::ssl::context::sslv23) {

    auto &config_manage = ConfigManage::instance();
    add_signals();
    ip::tcp::resolver resover(io_context);
    ip::tcp::endpoint endpoint = *resover.resolve(config_manage.server_cfg.local_addr,
                                                  config_manage.server_cfg.local_port).begin();
    load_server_certificate(ssl_context_);
    acceptor_.open(endpoint.protocol());
    acceptor_.set_option(tcp::acceptor::reuse_address(true));
    acceptor_.bind(endpoint);
    acceptor_.listen();
    if(config_manage.server_cfg.websocketEnabled)
    {
        NOTICE_LOG<<"listen websocket connection";
        do_websocket_accept();
    }
    else
    {
        do_accept();
    }

}
void Service::do_websocket_accept()
{
    websocket_connection_.reset(new WebsocketSession(context_pool.get_io_context(), ssl_context_));
    acceptor_.async_accept(websocket_connection_->socket(), [this](const boost::system::error_code &ec) {
        if(!ec){
            auto ep = websocket_connection_->socket().remote_endpoint();
            NOTICE_LOG << "accept incoming connection :" << ep.address().to_string();
            websocket_connection_->start();
        }
        else
        {
            NOTICE_LOG << "accept incoming connection fail:" << ec.message() << std::endl;

        }
        do_websocket_accept();

});
}

void Service::load_server_certificate(boost::asio::ssl::context &ctx) {

    auto &config_manage = ConfigManage::instance();

    ctx.set_options(
            boost::asio::ssl::context::default_workarounds
            | boost::asio::ssl::context::no_sslv2);
    // | boost::asio::ssl::context::single_dh_use);

    ssl_context_.use_certificate_chain_file(config_manage.server_cfg.certificate_chain);
    ssl_context_.use_private_key_file(config_manage.server_cfg.server_private_key, boost::asio::ssl::context::pem);
}


void Service::do_accept() {
    new_connection_.reset(new TlsSession(context_pool.get_io_context(), ssl_context_));
    acceptor_.async_accept(new_connection_->socket(), [this](const boost::system::error_code &ec) {
        auto clean_up = [this]() {
            if (new_connection_->socket().is_open()) {
                new_connection_->socket().close();
            }
        };
        if (!acceptor_.is_open()) {
            NOTICE_LOG << "Acceptor socket is not open, stop calling myself";
            clean_up();
            return;
        }
        if (ec == boost::asio::error::operation_aborted) {
            NOTICE_LOG << "got cancel signal, stop calling myself";
            clean_up();
            return;
        }
        if (!ec) {
            boost::system::error_code error;
            auto ep = new_connection_->socket().remote_endpoint(error);
            if (!error) {
                DEBUG_LOG << "accept incoming connection :" << ep.address().to_string();
                new_connection_->socket().set_option(boost::asio::socket_base::keep_alive(true));
                new_connection_->start();

            } else {
                NOTICE_LOG << "get remote point error :" << error.message();
                clean_up();
            }
        } else {
            // dump_current_open_fd();
            NOTICE_LOG << "accept incoming connection fail:" << ec.message() << std::endl;
            clean_up();
        }

        do_accept();
    });
}

void Service::run() {
    NOTICE_LOG << "SslServer start..." << std::endl;
    context_pool.run();
}

void Service::add_signals() {
    signals.add(SIGINT);
    signals.add(SIGTERM);
#ifdef SIGQUIT
    signals.add(SIGQUIT);
#endif
    signals.async_wait([this](const boost::system::error_code &ec, int sig) {
        // dump_current_open_fd();
        context_pool.stop();

        NOTICE_LOG << "Recieve signal:" << sig << " SslServer stopped..." << std::endl;
    });
}