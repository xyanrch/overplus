#include "Session.h"
#include "Protocol/UDPPacket.h"
#include "Shared/ConfigManage.h"
#include "Shared/Log.h"
#include <chrono>
#include <cstring>
#include <string>
template<class T>
Session<T>::Session(boost::asio::io_context& ioctx, boost::asio::ssl::context& sslctx, Session::Type type)

    : io_context_(ioctx)
    , upstream_socket(ioctx, sslctx)
    , downstream_socket(ioctx)
    , resolver_(ioctx)
    , udp_resolver(ioctx)
    , downstream_udp_socket(ioctx)
    , in_buf(MAX_BUFF_SIZE)
    , out_buf(MAX_BUFF_SIZE)
    , session_type(type)

{
}
template<class T>
void Session<T>::handle_custom_protocol()
{
    auto self = this->shared_from_this();
    upstream_socket.async_read_some(boost::asio::buffer(in_buf),
        [this, self](boost::system::error_code ec, std::size_t length) {
            if (ec) {
                NOTICE_LOG << " Read trojan message failed:" << ec.message();
                // Log::log_with_endpoint(in_endpoint, "SSL handshake failed: " + error.message(), Log::ERROR);
                destroy();
                return;
            }
            bool valid = false;
            if (VRequest::is_v_protocol(in_buf)) {

                valid = v_req.unstream(std::string(in_buf.data(), length));
                password = v_req.password;
                vprotocol = true;
            } else {
                valid = trojanReq.parse(std::string(in_buf.data(), length)) != -1;
                password = trojanReq.password;
            }

            if (valid) {
                //
                if (!ConfigManage::instance().server_cfg.allowed_passwords.count(password)) {
                    ERROR_LOG << "unsupported password from client....,end session";
                    destroy();
                    return;
                }

            } else {
                ERROR_LOG << "parse trojan request fail";
                destroy();
                return;
            }
            if (!vprotocol && trojanReq.command == TrojanReq::UDP_ASSOCIATE) {
                upstream_udp_buff = trojanReq.payload;
                handle_trojan_udp_proxy();

            } else {
                do_resolve();
            }
        });
}
template<class T>
void Session<T>::udp_upstream_read()
{
    auto self = this->shared_from_this();
    upstream_socket.async_read_some(boost::asio::buffer(in_buf),
        [this, self](boost::system::error_code ec, std::size_t length) {
            if (!ec) {
                upstream_udp_buff += std::string(in_buf.data(), length);
                handle_trojan_udp_proxy();

            } else {
                destroy();
            }
        });
}
template<class T>
void Session<T>::handle_trojan_udp_proxy()
{
    state_ = FORWARD;
    UDPPacket udp_packet;
    size_t packet_len;
    bool is_packet_valid = udp_packet.parse(upstream_udp_buff, packet_len);
    if (!is_packet_valid) {
        if (upstream_udp_buff.length() > MAX_BUFF_SIZE) {
            ERROR_LOG << "parse packet get wrong UDP packet too long";
            destroy();
            return;
        }
        udp_upstream_read();
        return;
    }
    upstream_udp_buff = upstream_udp_buff.substr(packet_len);
    DEBUG_LOG << "udp:" << udp_packet.address.address << ":" << udp_packet.address.port;
    auto self = this->shared_from_this();
    udp_resolver.async_resolve(udp_packet.address.address, std::to_string(udp_packet.address.port), [this, self, udp_packet](const boost::system::error_code error, const udp::resolver::results_type& results) {
        if (error || results.empty()) {
            ERROR_LOG << "cannot resolve remote server hostname " << udp_packet.address.address << ": " << error.message();
            destroy();
            return;
        }
        auto iterator = results.begin();
        for (auto it = results.begin(); it != results.end(); ++it) {
            const auto& addr = it->endpoint().address();
            if (addr.is_v4()) {
                iterator = it;
                break;
            }
        }
        // Log::log_with_endpoint(in_endpoint, query_addr + " is resolved to " + iterator->endpoint().address().to_string(), Log::ALL);
        if (!downstream_udp_socket.is_open()) {
            auto protocol = iterator->endpoint().protocol();
            boost::system::error_code ec;
            downstream_udp_socket.open(protocol, ec);
            if (ec) {
                destroy();
                return;
            }
        }
        downstream_udp_socket.async_send_to(boost::asio::buffer(udp_packet.payload), *iterator,
            [this, self](boost::system::error_code ec, std::size_t length) {
                if (!ec)
                    udp_async_bidirectional_read(3);
                else {
                    if (ec != boost::asio::error::operation_aborted) {
                        ERROR_LOG << "Server-->RemoteWeb(udp):" << ec.message();
                    }
                    // Most probably client closed socket. Let's close both sockets and exit session.
                    destroy();
                    return;
                }
            });
        // udp_async_bidirectional_write(1, udp_packet.payload, iterator);
    });
}
template<class T>
void Session<T>::udp_async_bidirectional_read(int direction)
{
    auto self = this->shared_from_this();
    // We must divide reads by direction to not permit second read call on the same socket.
    if (direction & 0x01)
        upstream_socket.async_read_some(boost::asio::buffer(in_buf),
            [this, self](boost::system::error_code ec, std::size_t length) {
                if (!ec) {
                    DEBUG_LOG << "--> " << std::to_string(length) << " bytes";
                    UDPPacket udp_packet;
                    size_t packet_len;
                    upstream_udp_buff += std::string(in_buf.data(), length);
                    bool is_packet_valid = udp_packet.parse(upstream_udp_buff, packet_len);
                    if (is_packet_valid) {
                        upstream_udp_buff = upstream_udp_buff.substr(packet_len);

                        udp_resolver.async_resolve(udp_packet.address.address, std::to_string(udp_packet.address.port), [this, self, udp_packet](const boost::system::error_code error, const udp::resolver::results_type& results) {
                            if (error || results.empty()) {
                                ERROR_LOG << "cannot resolve remote server hostname " << udp_packet.address.address << ": " << error.message();
                                destroy();
                                return;
                            }
                            auto iterator = results.begin();
                            for (auto it = results.begin(); it != results.end(); ++it) {
                                const auto& addr = it->endpoint().address();
                                if (addr.is_v4()) {
                                    iterator = it;
                                    break;
                                }
                            }

                            udp_async_bidirectional_write(1, udp_packet.payload, iterator);
                        });
                    } else {
                        if (upstream_udp_buff.length() > MAX_BUFF_SIZE) {
                            ERROR_LOG << "parse packet get wrong UDP packet too long";
                            destroy();
                            return;
                        }
                        udp_async_bidirectional_read(1);
                    }

                } else // if (ec != boost::asio::error::eof)
                {
                    if (ec != boost::asio::error::eof && ec != boost::asio::error::operation_aborted) {
                        ERROR_LOG << "client-->Server(UDP over tls) " << ec.message();
                    }

                    // Most probably client closed socket. Let's close both sockets and exit session.
                    destroy();
                    return;
                }
            });

    if (direction & 0x2) {
        udp::endpoint udp_sender_endpoint;
        downstream_udp_socket.async_receive_from(boost::asio::buffer(out_buf), udp_sender_endpoint,
            [this, self, udp_sender_endpoint](boost::system::error_code ec, std::size_t length) {
                if (!ec) {

                    DEBUG_LOG << "<-- " << std::to_string(length) << " bytes";
                    auto packet = UDPPacket::generate(udp_sender_endpoint, std::string(out_buf.data(), length));

                    udp_async_bidirectional_write(2, packet, boost::asio::ip::udp::resolver::iterator());
                } else // if (ec != boost::asio::error::eof)
                {
                    if (ec != boost::asio::error::eof && ec != boost::asio::error::operation_aborted) {
                        ERROR_LOG << "Server<--RemoteWeb(udp): " << ec.message();
                    }

                    // Most probably remote server closed socket. Let's close both sockets and exit session.
                    destroy();
                    return;
                }
            });
    }
}
template<class T>
void Session<T>::udp_async_bidirectional_write(int direction, const std::string& packet, boost::asio::ip::udp::resolver::iterator udp_ep)
{
    auto self(this->shared_from_this());

    switch (direction) {
    case 1:
        downstream_udp_socket.async_send_to(boost::asio::buffer(packet), *udp_ep,
            [this, self, direction](boost::system::error_code ec, std::size_t length) {
                if (!ec)
                    udp_async_bidirectional_read(direction);
                else {
                    if (ec != boost::asio::error::operation_aborted) {
                        ERROR_LOG << "Server-->RemoteWeb(udp):" << ec.message();
                    }
                    // Most probably client closed socket. Let's close both sockets and exit session.
                    destroy();
                    return;
                }
            });
        break;
    case 2:
        upstream_udp_write(direction, packet);
        break;
    }
}
template<class T>
void Session<T>::do_resolve()
{
    auto self(this->shared_from_this());
    remote_host = vprotocol ? v_req.address : trojanReq.address.address;
    remote_port = std::to_string(vprotocol ? v_req.port : trojanReq.address.port);
    resolver_.async_resolve(tcp::resolver::query(remote_host, remote_port),
        [this, self](const boost::system::error_code& ec, tcp::resolver::iterator it) {
            if (!ec) {
                do_connect(it);
            } else {
                ERROR_LOG << "failed to resolve " << remote_host << ":" << remote_port << " " << ec.message();
                destroy();
            }
        });
}
template<class T>
void Session<T>::do_connect(tcp::resolver::iterator& it)
{
    auto self(this->shared_from_this());
    state_ = FORWARD;
    downstream_socket.async_connect(*it,
        [this, self, it](const boost::system::error_code& ec) {
            if (!ec) {
                boost::asio::socket_base::keep_alive option(true);
                downstream_socket.set_option(option);
                DEBUG_LOG << "connected to " << remote_host << ":" << remote_port;

                if (vprotocol && !v_req.packed_buff.empty() || !vprotocol && !trojanReq.payload.empty()) {
                    DEBUG_LOG << "payload not empty";

                    boost::asio::async_write(downstream_socket, boost::asio::buffer(vprotocol ? v_req.packed_buff : trojanReq.payload),
                        [this, self](boost::system::error_code ec, std::size_t length) {
                            if (!ec)
                                async_bidirectional_read(3);
                            else {
                                ERROR_LOG << "Client<--Server:" << ec.message();
                                // Most probably client closed socket. Let's close both sockets and exit session.
                                destroy();
                                return;
                            }
                        });
                }
                // read packet from both direction
                else
                    async_bidirectional_read(3);

            } else {
                ERROR_LOG << "failed to connect " << remote_host << ":" << remote_port << " " << ec.message();
                destroy();
            }
        });
}
template<class T>
void Session<T>::async_bidirectional_read(int direction)
{
    auto self = this->shared_from_this();
    // We must divide reads by direction to not permit second read call on the same socket.
    if (direction & 0x01)
        upstream_socket.async_read_some(boost::asio::buffer(in_buf),
            [this, self](boost::system::error_code ec, std::size_t length) {
                if (!ec) {
                    DEBUG_LOG << "--> " << std::to_string(length) << " bytes";

                    async_bidirectional_write(1, length);
                } else // if (ec != boost::asio::error::eof)
                {
                    if (ec != boost::asio::error::eof && ec != boost::asio::error::operation_aborted) {
                        ERROR_LOG << "Client-->Server: " << ec.message();
                    }

                    // Most probably client closed socket. Let's close both sockets and exit session.
                    destroy();
                    return;
                }
            });

    if (direction & 0x2)
        downstream_socket.async_read_some(boost::asio::buffer(out_buf),
            [this, self](boost::system::error_code ec, std::size_t length) {
                if (!ec) {

                    DEBUG_LOG << "<-- " << std::to_string(length) << " bytes";

                    async_bidirectional_write(2, length);
                } else // if (ec != boost::asio::error::eof)
                {
                    if (ec != boost::asio::error::eof && ec != boost::asio::error::operation_aborted) {
                        ERROR_LOG << "Server<--RemoteWeb: " << ec.message();
                    }

                    // Most probably remote server closed socket. Let's close both sockets and exit session.
                    destroy();
                    return;
                }
            });
}
template<class T>
void Session<T>::async_bidirectional_write(int direction, size_t len)
{
    auto self =this->shared_from_this();

    switch (direction) {
    case 1:
        boost::asio::async_write(downstream_socket,boost::asio::buffer(in_buf, len),
            [this, self, direction](boost::system::error_code ec, std::size_t length) {
                if (!ec)
                    async_bidirectional_read(direction);
                else {
                    if (ec != boost::asio::error::operation_aborted) {
                        ERROR_LOG << "Server-->Webserver:" << ec.message();
                    }
                    // Most probably client closed socket. Let's close both sockets and exit session.
                    destroy();
                    return;
                }
            });
        break;
    case 2:
        upstream_tcp_write(direction, len);
        break;
    }
}

template<class T>
void Session<T>::upstream_tcp_write(int direction, size_t len)
{
    assert(0);
}
template<class T>
void Session<T>::upstream_udp_write(int direction, const std::string& packet)
{
    assert(0);
}
template<class T>
void Session<T>::destroy()
{
    DEBUG_LOG<<"Session destroyed called";
    boost::system::error_code ec;
    if (downstream_udp_socket.is_open()) {
        downstream_udp_socket.cancel(ec);
        downstream_udp_socket.close();
    }
    if (downstream_socket.is_open()) {
        downstream_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
        downstream_socket.cancel();
        downstream_socket.close();
    }
}