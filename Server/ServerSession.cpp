
// The following is a flow and structural diagram depicting the
// various elements  (proxy, server  and client)  and how  they
// connect and interact with each other.

//
//                                    ---> upstream --->           +---------------+
//                                                     +---->------>               |
//                               +-----------+         |           | Remote Server |
//                     +--------->          [x]--->----+  +---<---[x]              |
//                     |         | TCP Proxy |            |        +---------------+
// +-----------+       |  +--<--[x] Server   <-----<------+
// |          [x]--->--+  |      +-----------+
// |  Client   |          |
// |           <-----<----+
// +-----------+
//                <--- downstream <---
#include "ServerSession.h"

std::atomic<uint32_t> ServerSession::connection_num(0);
ServerSession::ServerSession(boost::asio::io_context& ioctx, boost::asio::ssl::context& sslctx)
    : Session<SSLSocket>(ioctx, sslctx, Session::TCP)
{
    connection_num++;
}
ServerSession::~ServerSession()
{
    connection_num--;
    NOTICE_LOG << "Session destructed, current alive session:" << connection_num.load();
}
void ServerSession::start()
{
    auto self = shared_from_this();
    upstream_ssl_socket.async_handshake(boost::asio::ssl::stream_base::server, [this, self](const boost::system::error_code& error) {
        if (error) {
            ERROR_LOG << "SSL handshake failed: " << error.message();
            destroy();
            return;
        }
        handle_custom_protocol();
    });
}

boost::asio::ip::tcp::socket& ServerSession::socket()
{
    return upstream_ssl_socket.next_layer();
}

void ServerSession::upstream_tcp_write(int direction, size_t len)
{
    auto self(this->shared_from_this());
    upstream_ssl_socket.async_write_some(boost::asio::buffer(out_buf, len), [this, self, direction](boost::system::error_code ec, std::size_t length) {
        if (!ec)
            async_bidirectional_read(direction);
        else {
            if (ec != boost::asio::error::operation_aborted) {
                ERROR_LOG << " closing session. Remote socket write error", ec.message();
            }
            // Most probably remote server closed socket. Let's close both sockets and exit session.
            destroy();
            return;
        }
    });
}

void ServerSession::upstream_udp_write(int direction, const std::string& packet)
{
    auto self(this->shared_from_this());
    upstream_ssl_socket.async_write_some(boost::asio::buffer(packet),
        [this, self, direction](boost::system::error_code ec, std::size_t length) {
            if (!ec)
                udp_async_bidirectional_read(direction);
            else {
                if (ec != boost::asio::error::operation_aborted) {
                    ERROR_LOG << " closing session. Remote socket write error", ec.message();
                }
                // Most probably remote server closed socket. Let's close both sockets and exit session.
                destroy();
                return;
            }
        });
}
void ServerSession::destroy()
{
    if (state_ == DESTROY) {
        return;
    }
    state_ = DESTROY;
    Session<SSLSocket>::destroy();
    if (upstream_ssl_socket.lowest_layer().is_open()) {
        boost::system::error_code ec;
        upstream_ssl_socket.lowest_layer().cancel(ec);
        upstream_ssl_socket.lowest_layer().shutdown(tcp::socket::shutdown_both, ec);
        upstream_ssl_socket.lowest_layer().close(ec);
    }
}