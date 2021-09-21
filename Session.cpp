
#include "Session.h"
#include <boost/asio/buffer.hpp>
#include <boost/asio/io_context.hpp>
#include <cstddef>
Session::Session(boost::asio::io_context& context)
    : context_(context)
    , in_socket(context_)
    , out_socket(context_)
    , resolver_(context_)
    , in_buf(MAX_BUFF_SIZE)
    , out_buf(MAX_BUFF_SIZE)
{
}

void Session::start()
{
    state_ = HANDSHAKE;
    do_read();
}
void Session::do_read()
{
    switch (state_) {
    case HANDSHAKE:
        sock5_handshake();
        break;

    case FORWARD:
        sock5_read_packet(3);
        break;
    default:
        ERROR_LOG << "Wrong state:" << state_;
    }
}
void Session::sock5_handshake()
{
    auto self(shared_from_this());
    in_socket.async_read_some(boost::asio::buffer(in_buf), [self, this](const boost::system::error_code& ec, size_t len) {
        if (!ec) {
            if (len < 3 || in_buf[0] != 0x05) {
                ERROR_LOG << "SOCKS5 handshake request is invalid. Closing session";
                return;
            }
            auto num_method = in_buf[1];
            in_buf[1] = 0xff;

            for (int i = 0; i < num_method; i++) {
                //currently only support 0x00 - 'NO AUTHENTICATION REQUIRED'
                if (in_buf[2 + i] == 0x00) {
                    in_buf[1] = 0x00;
                    break;
                }
            }
            write_sock5_hanshake_reply();

        } else {
            ERROR_LOG << "sock5 handshake error：" << ec.message();
            destroy();
        }
    });
}
void Session::write_sock5_hanshake_reply()
{
    auto self(shared_from_this());

    boost::asio::async_write(in_socket, boost::asio::buffer(in_buf, 2), // Always 2-byte according to RFC1928
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
                /*
The SOCKS request is formed as follows:
+----+-----+-------+------+----------+----------+
|VER | CMD |  RSV  | ATYP | DST.ADDR | DST.PORT |
+----+-----+-------+------+----------+----------+
| 1  |  1  | X'00' |  1   | Variable |    2     |
+----+-----+-------+------+----------+----------+
Where:
o  VER    protocol version: X'05'
o  CMD
o  CONNECT X'01'
o  BIND X'02'
o  UDP ASSOCIATE X'03'
o  RSV    RESERVED
o  ATYP   address type of following address
o  IP V4 address: X'01'
o  DOMAINNAME: X'03'
o  IP V6 address: X'04'
o  DST.ADDR       desired destination address
o  DST.PORT desired destination port_ in network octet
order
The SOCKS server will typically evaluate the request based on source
and destination addresses, and return one or more reply messages, as
appropriate for the request type.
*/
                if (length < 5 || in_buf[0] != 0x05 || in_buf[1] != 0x01) {
                    ERROR_LOG << "SOCKS5 request is invalid. Closing session.";
                    return;
                }

                uint8_t addr_type = in_buf[3], host_length;

                switch (addr_type) {
                case 0x01: // IP V4 addres
                    if (length != 10) {
                        ERROR_LOG << "SOCKS5 request length is invalid. Closing session.";
                        return;
                    }
                    remote_host = boost::asio::ip::address_v4(ntohl(*((uint32_t*)&in_buf[4]))).to_string();
                    remote_port = std::to_string(ntohs(*((uint16_t*)&in_buf[8])));
                    break;
                case 0x03: // DOMAINNAME
                    host_length = in_buf[4];
                    if (length != (size_t)(5 + host_length + 2)) {
                        ERROR_LOG << "SOCKS5 request length is invalid. Closing session.";
                        return;
                    }
                    remote_host = std::string(&in_buf[5], host_length);
                    remote_port = std::to_string(ntohs(*((uint16_t*)&in_buf[5 + host_length])));
                    break;
                default:
                    ERROR_LOG << "unsupport_ed address type in SOCKS5 request. Closing session.";
                    break;
                }

                do_resolve();
            } else {
                ERROR_LOG << "SOCKS5 request read:" << ec.message();
                destroy();
            }
            // ERROR_LOG << "SOCKS5 request read:" << ec.message();
        });
}
void Session::do_resolve()
{
    auto self(shared_from_this());

    resolver_.async_resolve(tcp::resolver::query({ remote_host, remote_port }),
        [this, self](const boost::system::error_code& ec, tcp::resolver::iterator it) {
            if (!ec) {
                do_connect(it);
            } else {
                ERROR_LOG << "failed to resolve " << remote_host << ":" << remote_port << " " << ec.message();
                destroy();
            }
        });
}
void Session::do_connect(tcp::resolver::iterator& it)
{
    auto self(shared_from_this());
    out_socket.async_connect(*it,
        [this, self](const boost::system::error_code& ec) {
            if (!ec) {
                DEBUG_LOG << "connected to " << remote_host << ":" << remote_port;
                write_socks5_response();
            } else {
                ERROR_LOG << "failed to connect " << remote_host << ":" << remote_port << " " << ec.message();
                destroy();
            }
        });
}

void Session::write_socks5_response()
{
    auto self(shared_from_this());

    /*
The SOCKS request information is sent by the client as soon as it has
established a connection to the SOCKS server, and completed the
authentication negotiations.  The server evaluates the request, and
returns a reply formed as follows:
+----+-----+-------+------+----------+----------+
|VER | REP |  RSV  | ATYP | BND.ADDR | BND.PORT |
+----+-----+-------+------+----------+----------+
| 1  |  1  | X'00' |  1   | Variable |    2     |
+----+-----+-------+------+----------+----------+
Where:
o  VER    protocol version: X'05'
o  REP    Reply field:
o  X'00' succeeded
o  X'01' general SOCKS server failure
o  X'02' connection not allowed by ruleset
o  X'03' Network unreachable
o  X'04' Host unreachable
o  X'05' Connection refused
o  X'06' TTL expired
o  X'07' Command not support_ed
o  X'08' Address type not support_ed
o  X'09' to X'FF' unassigned
o  RSV    RESERVED
o  ATYP   address type of following address
o  IP V4 address: X'01'
o  DOMAINNAME: X'03'
o  IP V6 address: X'04'
o  BND.ADDR       server bound address
o  BND.PORT       server bound port_ in network octet order
Fields marked RESERVED (RSV) must be set to X'00'.
*/
    in_buf[0] = 0x05;
    in_buf[1] = 0x00;
    in_buf[2] = 0x00;
    in_buf[3] = 0x01;
    uint32_t realRemoteIP = out_socket.remote_endpoint().address().to_v4().to_ulong();
    uint16_t realRemoteport = htons(out_socket.remote_endpoint().port());

    std::memcpy(&in_buf[4], &realRemoteIP, 4);
    std::memcpy(&in_buf[8], &realRemoteport, 2);

    boost::asio::async_write(in_socket, boost::asio::buffer(in_buf, 10), // Always 10-byte according to RFC1928
        [this, self](boost::system::error_code ec, std::size_t length) {
            if (!ec) {
                state_ = FORWARD;
                do_read(); // Read both sockets
            } else
                ERROR_LOG << "SOCKS5 response write:" << ec.message();
        });
}
void Session::sock5_read_packet(int direction)
{
    auto self(shared_from_this());

    // We must divide reads by direction to not permit second read call on the same socket.
    if (direction & 0x01)
        in_socket.async_read_some(boost::asio::buffer(in_buf),
            [this, self](boost::system::error_code ec, std::size_t length) {
                if (!ec) {
                    DEBUG_LOG << "--> " << std::to_string(length) << " bytes";

                    sock5_write_packet(1, length);
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

                    sock5_write_packet(2, length);
                } else //if (ec != boost::asio::error::eof)
                {
                    ERROR_LOG << "closing session. Remote socket read error" << ec.message();
                    // Most probably remote server closed socket. Let's close both sockets and exit session.
                    destroy();
                    //context_.stop();
                }
            });
}
void Session::sock5_write_packet(int direction, size_t len)
{
    auto self(shared_from_this());

    switch (direction) {
    case 1:
        boost::asio::async_write(out_socket, boost::asio::buffer(in_buf, len),
            [this, self, direction](boost::system::error_code ec, std::size_t length) {
                if (!ec)
                    sock5_read_packet(direction);
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
                    sock5_read_packet(direction);
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

    if (out_socket.is_open()) {
        auto self = shared_from_this();

        out_socket.cancel(ec);
        out_socket.shutdown(tcp::socket::shutdown_both, ec);
        out_socket.close();
        //  ssl_shutdown_timer.expires_after(chrono::seconds(SSL_SHUTDOWN_TIMEOUT));
        // ssl_shutdown_timer.async_wait(ssl_shutdown_cb);
    }
}