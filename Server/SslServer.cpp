#include "SslServer.h"
#include <boost/asio/io_context.hpp>
#include <cstdlib>
#include <cstring>
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
    log_level = config_manage.server_cfg.log_level;
    add_signals();
    ip::tcp::resolver resover(io_context);
    ip::tcp::endpoint endpoint = *resover.resolve(config_manage.server_cfg.local_addr, config_manage.server_cfg.local_port).begin();
    //
    ssl_context_.set_options(
        boost::asio::ssl::context::default_workarounds
        | boost::asio::ssl::context::no_sslv2
        | boost::asio::ssl::context::single_dh_use);
    // ssl_context_.set_password_callback([this](auto size, auto purpose) {
    //    return "";
    // });
    ssl_context_.use_certificate_chain_file(config_manage.server_cfg.certificate_chain);
    ssl_context_.use_private_key_file(config_manage.server_cfg.server_private_key, boost::asio::ssl::context::pem);
    //ssl_context_.use_tmp_dh_file("dh512.pem");
    //
    // ssl_context_.use_tmp_dh(boost::asio::const_buffer(g_dh2048_sz, g_dh2048_sz_size));

    auto native_context = ssl_context_.native_handle();
    /*const char* config = "http/1.1";
    SSL_CTX_set_alpn_select_cb(
        native_context, [](SSL*, const unsigned char** out, unsigned char* outlen, const unsigned char* in, unsigned int inlen, void* config) -> int {
            if (SSL_select_next_proto((unsigned char**)out, outlen, (unsigned char*)(config), std::strlen((const char*)config), in, inlen) != OPENSSL_NPN_NEGOTIATED) {
                return SSL_TLSEXT_ERR_NOACK;
            }
            return SSL_TLSEXT_ERR_OK;
        },
        &config);*/
    acceptor_.open(endpoint.protocol());
    acceptor_.set_option(ip::tcp::acceptor::reuse_address(true));
    acceptor_.bind(endpoint);
    acceptor_.listen();
    do_accept();
}
void SslServer::do_accept()
{
    std::shared_ptr<ServerSession> new_session = std::make_shared<ServerSession>(context_pool.get_io_context(), ssl_context_);
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
        acceptor_.close();

        NOTICE_LOG << "SslServer stopped..." << std::endl;
        exit(1);
    });
}