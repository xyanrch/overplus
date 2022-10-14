#include "SslServer.h"
#include "Server/ServerSession.h"
#include "Shared/Log.h"
#include <boost/asio/io_context.hpp>
#include <boost/system/error_code.hpp>
#include <cstdlib>
#include <cstring>
//#include <filesystem>
#include <memory>
#include <string>

SslServer::SslServer()
    : context_pool(5)
    , io_context(context_pool.get_io_context())
    , signals(io_context)
    , acceptor_(io_context)
    , ssl_context_(boost::asio::ssl::context::sslv23)

{

    auto& config_manage = ConfigManage::instance();
    add_signals();
    ip::tcp::resolver resover(io_context);
    ip::tcp::endpoint endpoint = *resover.resolve(config_manage.server_cfg.local_addr, config_manage.server_cfg.local_port).begin();
    load_server_certificate(ssl_context_);
    acceptor_.open(endpoint.protocol());
    acceptor_.set_option(tcp::acceptor::reuse_address(true));
    acceptor_.bind(endpoint);
    acceptor_.listen();
    do_accept();
}
void SslServer::load_server_certificate(boost::asio::ssl::context& ctx)
{

    /*
        The certificate was generated from bash on Ubuntu (OpenSSL 1.1.1f) using:
        openssl dhparam -out dh.pem 2048
        openssl trojanReq -newkey rsa:2048 -nodes -keyout key.pem -x509 -days 10000 -out cert.pem -subj "/C=US/ST=CA/L=Los Angeles/O=Beast/CN=www.example.com"
       std::string const dh = "-----BEGIN DH PARAMETERS-----\n"
                             "MIIBCAKCAQEArzQc5mpm0Fs8yahDeySj31JZlwEphUdZ9StM2D8+Fo7TMduGtSi+\n"
                             "/HRWVwHcTFAgrxVdm+dl474mOUqqaz4MpzIb6+6OVfWHbQJmXPepZKyu4LgUPvY/\n"
                             "4q3/iDMjIS0fLOu/bLuObwU5ccZmDgfhmz1GanRlTQOiYRty3FiOATWZBRh6uv4u\n"
                             "tff4A9Bm3V9tLx9S6djq31w31Gl7OQhryodW28kc16t9TvO1BzcV3HjRPwpe701X\n"
                             "oEEZdnZWANkkpR/m/pfgdmGPU66S2sXMHgsliViQWpDCYeehrvFRHEdR9NV+XJfC\n"
                             "QMUk26jPTIVTLfXmmwU0u8vUkpR7LQKkwwIBAg==\n"
                             "-----END DH PARAMETERS-----\n";
            // passphrase from the privatekey
            ctx.set_password_callback(
          [](std::size_t,
              boost::asio::ssl::context_base::password_purpose) {
              return "test";
          });
    // ctx.use_tmp_dh(
    //   boost::asio::buffer(dh.data(), dh.size()));

  */
    auto& config_manage = ConfigManage::instance();

    ctx.set_options(
        boost::asio::ssl::context::default_workarounds
        | boost::asio::ssl::context::no_sslv2);
    // | boost::asio::ssl::context::single_dh_use);

    ssl_context_.use_certificate_chain_file(config_manage.server_cfg.certificate_chain);
    ssl_context_.use_private_key_file(config_manage.server_cfg.server_private_key, boost::asio::ssl::context::pem);
}
/*static void dump_current_open_fd()
{

    std::string path = "/proc/" + std::to_string(::getpid()) + "/fd/";
    // unsigned count = std::distance(std::filesystem::directory_iterator(path),
    //    std::filesystem::directory_iterator());

    //  NOTICE_LOG << "Current open fd count:" << count;
}*/
void SslServer::do_accept()
{
    new_connection_.reset(new ServerSession(context_pool.get_io_context(), ssl_context_));
    acceptor_.async_accept(new_connection_->socket(), [this](const boost::system::error_code& ec) {
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
                DEBUG_LOG << "Current alive sessions:" << ServerSession::connection_num.load() << "accept incoming connection :" << ep.address().to_string();
                boost::asio::socket_base::keep_alive option(true);
                new_connection_->socket().set_option(option);
                new_connection_->start();

            } else {
                NOTICE_LOG << "get remote point error :" << error.message();
                clean_up();
            }
        } else {
            // dump_current_open_fd();
            NOTICE_LOG << "Current alive sessions:" << ServerSession::connection_num.load() << "accept incoming connection fail:" << ec.message() << std::endl;
            clean_up();
        }

        do_accept();
    });
}
void SslServer::run()
{
    NOTICE_LOG << "SslServer start..." << std::endl;
    context_pool.run();
}
void SslServer::add_signals()
{
    signals.add(SIGINT);
    signals.add(SIGTERM);
#ifdef SIGQUIT
    signals.add(SIGQUIT);
#endif
    signals.async_wait([this](const boost::system::error_code& ec, int sig) {
        // dump_current_open_fd();
        context_pool.stop();

        NOTICE_LOG << "Recieve signal:" << sig << " SslServer stopped..." << std::endl;
    });
}