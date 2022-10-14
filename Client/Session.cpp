
#include "Session.h"
#include "Shared/ConfigManage.h"
#include "Shared/Log.h"
#include <Protocal/VplusProtocal/VRequest.h>
#include <Protocal/socks5/socks5.h>
#include <boost/asio/buffer.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ssl/verify_mode.hpp>
#include <cstddef>
Session::Session(boost::asio::io_context& context, boost::asio::ssl::context& ssl)
    : context_(context)
    , in_socket(context_)

    , resolver_(context_)
    , in_buf(MAX_BUFF_SIZE)
    , out_buf(MAX_BUFF_SIZE)
    , ssl_ctx(ssl)
    , out_socket(context_, ssl_ctx)

{
    out_socket.set_verify_mode(boost::asio::ssl::verify_none);
}

void Session::start()
{
    state_ = HANDSHAKE;
    sock5_handshake();
}

void Session::sock5_handshake()
{
    auto self(shared_from_this());
    in_socket.async_read_some(boost::asio::buffer(in_buf), [self, this](const boost::system::error_code& ec, size_t len) {
        if (!ec) {

            AuthReq auth_req;
            if (auth_req.unstream(in_buf)) {
                NOTICE_LOG << "receive message:" << auth_req;
                write_sock5_hanshake_reply(auth_req);

            } else {
                ERROR_LOG << "Receive invalid message";
                destroy();
            }

        } else {
            ERROR_LOG << "sock5 handshake error:" << ec.message();
            destroy();
        }
    });
}
void Session::write_sock5_hanshake_reply(AuthReq& req)
{
    auto self(shared_from_this());
    auto it = std::find(req.methods.cbegin(), req.methods.cend(), AuthMethod::NO_AUTHENTICATION);
    if (it == req.methods.cend()) {
        ERROR_LOG << "Now only support no password auth";
        destroy();
    }
    {
        AuthRes authRes;
        authRes.version = 0x05;
        authRes.method = AuthMethod::NO_AUTHENTICATION;

        authRes.stream(message_buf);
    }
    boost::asio::async_write(in_socket, boost::asio::buffer(message_buf), // Always 2-byte according to RFC1928
        [this, self](boost::system::error_code ec, std::size_t length) {
            if (!ec) {
                if (in_buf[1] == (char)0xFF)
                    return; // No appropriate auth method found. Close session.
                read_socks5_request();
            } else {
                ERROR_LOG << "SOCKS5 handshake response write :" << ec.message();
                destroy();
            }
        });
}
void Session::read_socks5_request()
{
    auto self(shared_from_this());

    in_socket.async_read_some(boost::asio::buffer(in_buf),
        [this, self](boost::system::error_code ec, std::size_t length) {
            if (!ec) {
                Request req;
                if (!req.unsteam(in_buf, length)) {
                    ERROR_LOG << "decode mesage error";
                    destroy();
                    return;
                }
                //  do_connec_to_proxy_server(trojanReq);
                do_resolve(req);
            } else {
                ERROR_LOG << "SOCKS5 request read:" << ec.message();
                destroy();
            }
            // ERROR_LOG << "SOCKS5 request read:" << ec.message();
        });
}

void Session::do_resolve(const Request& req)
{
    auto self(shared_from_this());
    auto& config = ConfigManage::instance().client_cfg;
    resolver_.async_resolve(tcp::resolver::query(config.remote_addr, config.remote_port),
        [req, this, self](const boost::system::error_code& ec, tcp::resolver::iterator it) {
            if (!ec) {
                do_connect(it, req);
            } else {
                ERROR_LOG << "failed to resolve " << remote_host << ":" << remote_port << " " << ec.message();
                destroy();
            }
        });
}
void Session::do_connect(tcp::resolver::iterator& it, const Request& req)
{
    auto self(shared_from_this());
    //
    out_socket.lowest_layer().async_connect(*it,
        [req, this, self](const boost::system::error_code& ec) {
            if (!ec) {
                DEBUG_LOG << "connected to " << remote_host << ":" << remote_port;
                do_ssl_handshake(req);
            } else {
                ERROR_LOG << "failed to connect " << remote_host << ":" << remote_port << " " << ec.message();
                destroy();
            }
        });
}
void Session::do_ssl_handshake(const Request& req)
{
    auto self(shared_from_this());
    out_socket.async_handshake(boost::asio::ssl::stream_base::client, [req, this, self](const boost::system::error_code& error) {
        if (!error) {
            do_sent_v_req(req);
        } else {
            ERROR_LOG << "ssl handshake failed :" << error.message();
            destroy();
        }
    });
}
void Session::do_sent_v_req(const Request& req)
{
    auto self(shared_from_this());
    message_buf.clear();
    auto& config = ConfigManage::instance().client_cfg;
    VRequest request;

    request.header.version = 0x01;

    request.user_name = config.user_name;
    request.password = config.password;
    request.address = req.remote_host;
    request.port = req.remote_port;
    request.stream(message_buf);

    boost::asio::async_write(out_socket, boost::asio::buffer(message_buf), // Always 10-byte according to RFC1928
        [this, self](boost::system::error_code ec, std::size_t length) {
            if (!ec) {
                state_ = FORWARD;
                write_socks5_response(); // Read both sockets
            } else
                ERROR_LOG << "SOCKS5 response write:" << ec.message();
        });
}
void Session::write_socks5_response()
{
    auto self(shared_from_this());

    {
        Reply reply;
        reply.version = 0x05;
        reply.reserved = 0x00;
        reply.addrtype = ADDRTYPE::V4;
        reply.repResult = 0x00;
        reply.realRemoteIP = out_socket.lowest_layer().remote_endpoint().address().to_v4().to_ulong();
        reply.realRemotePort = htons(out_socket.lowest_layer().remote_endpoint().port());
        message_buf.clear();
        reply.stream(message_buf);
    }

    boost::asio::async_write(in_socket, boost::asio::buffer(message_buf), // Always 10-byte according to RFC1928
        [this, self](boost::system::error_code ec, std::size_t length) {
            if (!ec) {
                state_ = FORWARD;
                read_packet(3); // Read both sockets
            } else
                ERROR_LOG << "SOCKS5 response write:" << ec.message();
        });
}
void Session::read_packet(int direction)
{
    auto self(shared_from_this());

    // We must divide reads by direction to not permit second read call on the same socket.
    if (direction & 0x01)
        in_socket.async_read_some(boost::asio::buffer(in_buf),
            [this, self](boost::system::error_code ec, std::size_t length) {
                if (!ec) {
                    DEBUG_LOG << "--> " << std::to_string(length) << " bytes";

                    write_packet(1, length);
                } else // if (ec != boost::asio::error::eof)
                {
                    ERROR_LOG << "closing session. Client socket read error" << ec.message();
                    // Most probably client closed socket. Let's close both sockets and exit session.
                    destroy();
                    // context_.stop();
                }
            });

    if (direction & 0x2)
        out_socket.async_read_some(boost::asio::buffer(out_buf),
            [this, self](boost::system::error_code ec, std::size_t length) {
                if (!ec) {

                    DEBUG_LOG << "<-- " << std::to_string(length) << " bytes";

                    write_packet(2, length);
                } else // if (ec != boost::asio::error::eof)
                {
                    ERROR_LOG << "closing session. Remote socket read error" << ec.message();
                    // Most probably remote server closed socket. Let's close both sockets and exit session.
                    destroy();
                    // context_.stop();
                }
            });
}
void Session::write_packet(int direction, size_t len)
{
    auto self(shared_from_this());

    switch (direction) {
    case 1:
        boost::asio::async_write(out_socket, boost::asio::buffer(in_buf, len),
            [this, self, direction](boost::system::error_code ec, std::size_t length) {
                if (!ec)
                    read_packet(direction);
                else {
                    ERROR_LOG << "closing session. Client socket write error" << ec.message();
                    // Most probably client closed socket. Let's close both sockets and exit session.
                    destroy();
                }
            });
        break;
    case 2:
        boost::asio::async_write(in_socket, boost::asio::buffer(out_buf, len),
            [this, self, direction](boost::system::error_code ec, std::size_t length) {
                if (!ec)
                    read_packet(direction);
                else {
                    ERROR_LOG << "closing session. Remote socket write error", ec.message();
                    // Most probably remote server closed socket. Let's close both sockets and exit session.
                    destroy();
                }
            });
        break;
    }
}
boost::asio::ip::tcp::socket& Session::socket()
{
    return in_socket;
}
void Session::destroy()
{
    ERROR_LOG << "destroy session";
    // Log::log_with_endpoint(in_endpoint, "disconnected, " + to_string(recv_len) + " bytes received, " + to_string(sent_len) + " bytes sent, lasted for " + to_string(time(nullptr) - start_time) + " seconds", Log::INFO);
    boost::system::error_code ec;
    resolver_.cancel();

    if (in_socket.is_open()) {
        in_socket.cancel(ec);
        in_socket.shutdown(tcp::socket::shutdown_both, ec);
        in_socket.close(ec);
    }

    if (out_socket.lowest_layer().is_open()) {
        auto self = shared_from_this();

        out_socket.lowest_layer().shutdown(tcp::socket::shutdown_both, ec);
        out_socket.lowest_layer().close();
        //  ssl_shutdown_timer.expires_after(chrono::seconds(SSL_SHUTDOWN_TIMEOUT));
        // ssl_shutdown_timer.async_wait(ssl_shutdown_cb);
    }
}