#include "ServerSession.h"
#include "Log.h"
#include <cstring>
#include <string>

ServerSession::ServerSession(boost::asio::io_context& ioctx, boost::asio::ssl::context& sslctx)

    : io_context_(ioctx)
    , in_ssl_socket(ioctx, sslctx)
    , out_socket(ioctx)
    , resolver_(ioctx)
    , in_buf(MAX_BUFF_SIZE)
    , out_buf(MAX_BUFF_SIZE)

{
}
void ServerSession::start()
{
    auto self = shared_from_this();
    auto ep = in_ssl_socket.next_layer().remote_endpoint();
    // NOTICE_LOG << " start ep:" << ep.address().to_string();
    //ssl handshake
    in_ssl_socket.async_handshake(boost::asio::ssl::stream_base::server, [this, self](auto error) {
        if (error) {
            // Log::log_with_endpoint(in_endpoint, "SSL handshake failed: " + error.message(), Log::ERROR);
            destroy();
            return;
        }
        handle_trojan_handshake();
    });
}
void ServerSession::handle_trojan_handshake()
{
    //trojan handshak
    auto self = shared_from_this();
    //in_ssl_socket.next_layer().read_some(boost::asio::buffer)read_some()
    // char temp[4096];
    in_ssl_socket.async_read_some(boost::asio::buffer(in_buf),
        [this, self](boost::system::error_code ec, std::size_t length) {
            if (ec) {
                // Log::log_with_endpoint(in_endpoint, "SSL handshake failed: " + error.message(), Log::ERROR);
                destroy();
                return;
            }
            //NOTICE_LOG << "error code:" << ec.message() << "trojan request: len =" << length << " data:" << std::string(in_buf.data(), length);
            if (length == 0) {
                // handle_trojan_handshake();
                //   return;
            }
            bool valid = req.parse(std::string(in_buf.data(), length)) != -1;
            if (valid) {
                //
                //TODO verify password

                // if (req.password != "yanyun90") {
                //NOTICE_LOG << "unspported password:" << req.password;
                // destroy();
                //return;
                //  }
            } else {
                ERROR_LOG << "parse trojan request fail";
                return;
            }
            do_resolve();
        });
}
void ServerSession::do_resolve()
{
    auto self(shared_from_this());

    resolver_.async_resolve(tcp::resolver::query({ req.address.address, std::to_string(req.address.port) }),
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
    out_socket.async_connect(*it,
        [this, self, it](const boost::system::error_code& ec) {
            if (!ec) {
                DEBUG_LOG << "connected to " << req.address.address << ":" << req.address.port;
                // write_socks5_response();
                //TODO
                if (req.payload.empty() == false) {
                    DEBUG_LOG << "payload not empty";
                    //std::memcpy(out_buf.data(), req.payload.data(), req.payload.length());
                    // out_async_write(1, req.payload.length());

                    boost::asio::async_write(out_socket, boost::asio::buffer(req.payload),
                        [this, self](boost::system::error_code ec, std::size_t length) {
                            if (!ec)
                                in_async_read(3);
                            else {
                                ERROR_LOG << "closing session. Client socket write error" << ec.message();
                                // Most probably client closed socket. Let's close both sockets and exit session.
                                destroy();
                            }
                        });
                }
                //read packet from both dirction
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
                } else //if (ec != boost::asio::error::eof)
                {
                    ERROR_LOG << "closing session. Client socket read error" << ec.message();
                    // Most probably client closed socket. Let's close both sockets and exit session.
                    destroy();
                    //context_.stop();
                }
            });

    if (direction & 0x2)
        out_socket.async_read_some(boost::asio::buffer(out_buf),
            [this, self](boost::system::error_code ec, std::size_t length) {
                if (!ec) {

                    DEBUG_LOG << "<-- " << std::to_string(length) << " bytes";

                    out_async_write(2, length);
                } else //if (ec != boost::asio::error::eof)
                {
                    ERROR_LOG << "closing session. Remote socket read error" << ec.message();
                    // Most probably remote server closed socket. Let's close both sockets and exit session.
                    destroy();
                    //context_.stop();
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
    boost::system::error_code ec;
    //  resolver.cancel();
    if (out_socket.is_open()) {
        out_socket.cancel(ec);
        out_socket.shutdown(tcp::socket::shutdown_both, ec);
        out_socket.close(ec);
    }

    if (in_ssl_socket.next_layer().is_open()) {
        auto self = shared_from_this();
        auto ssl_shutdown_cb = [this, self](const boost::system::error_code error) {
            if (error == boost::asio::error::operation_aborted) {
                return;
            }
            boost::system::error_code ec;
            //ssl_shutdown_timer.cancel();
            in_ssl_socket.next_layer().cancel(ec);
            in_ssl_socket.next_layer().shutdown(tcp::socket::shutdown_both, ec);
            in_ssl_socket.next_layer().close(ec);
        };
        in_ssl_socket.next_layer().cancel(ec);
        in_ssl_socket.async_shutdown(ssl_shutdown_cb);
        // ssl_shutdown_timer.expires_after(chrono::seconds(SSL_SHUTDOWN_TIMEOUT));
        //ssl_shutdown_timer.async_wait(ssl_shutdown_cb);
    }
}