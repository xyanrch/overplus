#include "ServerSession.h"
#include "Shared/ConfigManage.h"
#include "Shared/Log.h"
#include <chrono>
#include <cstring>
#include <string>
constexpr static int SSL_SHUTDOWN_TIMEOUT = 2;

ServerSession::ServerSession(boost::asio::io_context& ioctx, boost::asio::ssl::context& sslctx)

    : io_context_(ioctx)
    , in_ssl_socket(ioctx, sslctx)
    , out_socket(ioctx)
    , resolver_(ioctx)
    , in_buf(MAX_BUFF_SIZE)
    , out_buf(MAX_BUFF_SIZE)
    , ssl_shutdown_timer(ioctx)

{
}
void ServerSession::start()
{
    auto self = shared_from_this();
    // NOTICE_LOG << " start ep:" << ep.address().to_string();
    // ssl handshake
    in_ssl_socket.async_handshake(boost::asio::ssl::stream_base::server, [this, self](const boost::system::error_code& error) {
        if (error) {
            ERROR_LOG << "SSL handshake failed: " << error.message();
            destroy();
            return;
        }
        handle_trojan_handshake();
    });
}
void ServerSession::handle_trojan_handshake()
{
    // trojan handshak
    auto self = shared_from_this();
    // in_ssl_socket.next_layer().read_some(boost::asio::buffer)read_some()
    //  char temp[4096];
    in_ssl_socket.async_read_some(boost::asio::buffer(in_buf),
        [this, self](boost::system::error_code ec, std::size_t length) {
            if (ec) {
                // Log::log_with_endpoint(in_endpoint, "SSL handshake failed: " + error.message(), Log::ERROR);
                destroy();
                return;
            }
            bool valid = req.parse(std::string(in_buf.data(), length)) != -1;
            if (valid) {
                //
                if (!ConfigManage::instance().server_cfg.allowed_passwords.count(req.password)) {
                    ERROR_LOG << "unspoorted password from client....,end session";
                    destroy();
                    return;
                }

            } else {
                ERROR_LOG << "parse trojan request fail";
                destroy();
                return;
            }
            do_resolve();
        });
}
void ServerSession::do_resolve()
{
    auto self(shared_from_this());

    resolver_.async_resolve(tcp::resolver::query(req.address.address, std::to_string(req.address.port)),
        [this, self](const boost::system::error_code& ec, tcp::resolver::iterator it) {
            if (!ec) {
                do_connect(it);
            } else {
                ERROR_LOG << "failed to resolve " << remote_host << ":" << remote_port << " " << ec.message();
                destroy();
            }
        });
}

void ServerSession::do_connect(tcp::resolver::iterator& it)
{
    auto self(shared_from_this());
    state_ = FORWARD;
    out_socket.async_connect(*it,
        [this, self, it](const boost::system::error_code& ec) {
            if (!ec) {
                DEBUG_LOG << "connected to " << req.address.address << ":" << req.address.port;
                // write_socks5_response();
                // TODO
                if (req.payload.empty() == false) {
                    DEBUG_LOG << "payload not empty";
                    // std::memcpy(out_buf.data(), req.payload.data(), req.payload.length());
                    //  out_async_write(1, req.payload.length());

                    boost::asio::async_write(out_socket, boost::asio::buffer(req.payload),
                        [this, self](boost::system::error_code ec, std::size_t length) {
                            if (!ec)
                                in_async_read(3);
                            else {
                                ERROR_LOG << "closing session. Client socket write error" << ec.message();
                                // Most probably client closed socket. Let's close both sockets and exit session.
                                destroy();
                                return;
                            }
                        });
                }
                // read packet from both dirction
                else
                    in_async_read(3);

            } else {
                ERROR_LOG << "failed to connect " << remote_host << ":" << remote_port << " " << ec.message();
                destroy();
            }
        });
}
void ServerSession::in_async_read(int direction)
{
    auto self = shared_from_this();
    // We must divide reads by direction to not permit second read call on the same socket.
    if (direction & 0x01)
        in_ssl_socket.async_read_some(boost::asio::buffer(in_buf),
            [this, self](boost::system::error_code ec, std::size_t length) {
                if (!ec) {
                    DEBUG_LOG << "--> " << std::to_string(length) << " bytes";

                    out_async_write(1, length);
                } else // if (ec != boost::asio::error::eof)
                {
                    ERROR_LOG << "closing session. Client socket read error: " << ec.message();
                    // Most probably client closed socket. Let's close both sockets and exit session.
                    destroy();
                    return;
                    // context_.stop();
                }
            });

    if (direction & 0x2)
        out_socket.async_read_some(boost::asio::buffer(out_buf),
            [this, self](boost::system::error_code ec, std::size_t length) {
                if (!ec) {

                    DEBUG_LOG << "<-- " << std::to_string(length) << " bytes";

                    out_async_write(2, length);
                } else // if (ec != boost::asio::error::eof)
                {
                    ERROR_LOG << "closing session. Remote socket read error: " << ec.message();
                    // Most probably remote server closed socket. Let's close both sockets and exit session.
                    destroy();
                    return;
                    // context_.stop();
                }
            });
}
void ServerSession::out_async_write(int direction, size_t len)
{
    auto self(shared_from_this());

    switch (direction) {
    case 1:
        boost::asio::async_write(out_socket, boost::asio::buffer(in_buf, len),
            [this, self, direction](boost::system::error_code ec, std::size_t length) {
                if (!ec)
                    in_async_read(direction);
                else {
                    ERROR_LOG << "closing session. Client socket write error" << ec.message();
                    // Most probably client closed socket. Let's close both sockets and exit session.
                    destroy();
                    return;
                }
            });
        break;
    case 2:
        boost::asio::async_write(in_ssl_socket, boost::asio::buffer(out_buf, len),
            [this, self, direction](boost::system::error_code ec, std::size_t length) {
                if (!ec)
                    in_async_read(direction);
                else {
                    ERROR_LOG << "closing session. Remote socket write error", ec.message();
                    // Most probably remote server closed socket. Let's close both sockets and exit session.
                    destroy();
                    return;
                }
            });
        break;
    }
}
boost::asio::ip::tcp::socket& ServerSession::socket()
{
    return in_ssl_socket.next_layer();
}
void ServerSession::destroy()
{
    if (state_ == DESTROY) {
        return;
    }
    state_ = DESTROY;

    boost::system::error_code ec;
    resolver_.cancel();
    if (out_socket.is_open()) {
        out_socket.cancel(ec);
        out_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
        out_socket.close();
    }
    if (in_ssl_socket.lowest_layer().is_open()) {
        auto self = shared_from_this();
        in_ssl_socket.lowest_layer().cancel(ec);
        ssl_shutdown_timer.expires_after(std::chrono::seconds(SSL_SHUTDOWN_TIMEOUT));
        in_ssl_socket.async_shutdown([this, self](const boost::system::error_code error) {
            if (error == boost::asio::error::operation_aborted) {
                return;
            }

            boost::system::error_code ec;
            ssl_shutdown_timer.cancel();
            in_ssl_socket.lowest_layer().cancel(ec);
            in_ssl_socket.lowest_layer().shutdown(tcp::socket::shutdown_both, ec);
            in_ssl_socket.lowest_layer().close(ec);
        });
        ssl_shutdown_timer.async_wait([this, self](const boost::system::error_code error) {
            if (error == boost::asio::error::operation_aborted) {
                return;
            }
            if (in_ssl_socket.lowest_layer().is_open()) {
                boost::system::error_code ec;
                in_ssl_socket.lowest_layer().cancel(ec);
                in_ssl_socket.lowest_layer().shutdown(tcp::socket::shutdown_both, ec);
                in_ssl_socket.lowest_layer().close(ec);
            }
        });
    }
}