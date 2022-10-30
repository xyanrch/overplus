#pragma once

#include "Protocol/UDPPacket.h"
#include "Shared/Log.h"
#include <Protocol/TrojanReq.h>
#include <Protocol/VProtocal/VRequest.h>
#include <atomic>
#include <boost/asio.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/core/noncopyable.hpp>
#include <boost/optional.hpp>
#include <boost/system/error_code.hpp>
#include <cstdint>
#include <ctime>
#include <memory>
class UDPPacket;

using SSLSocket = boost::asio::ssl::stream<boost::asio::ip::tcp::socket>;
using boost::asio::ip::tcp;
using boost::asio::ip::udp;

class ServerSession : public std::enable_shared_from_this<ServerSession>
    , private boost::noncopyable {
    enum State {
        HANDSHAKE,
        FORWARD,
        DESTROY
    };

public:
    ServerSession(boost::asio::io_context&, boost::asio::ssl::context&);

    ~ServerSession();

    void start();

    void do_read();

    boost::asio::ip::tcp::socket& socket();

    // void handle_sock5();
    void async_bidirectional_read(int direction);

    void handle_custom_protocol();

    void do_resolve();

    void do_connect(tcp::resolver::iterator&);
    void udp_upstream_read();

    void async_bidirectional_write(int, size_t);
    void handle_trojan_udp_proxy();
    void udp_async_bidirectional_read(int direction);
    void udp_async_bidirectional_write(int, const std::string&, boost::asio::ip::udp::resolver::iterator);

    void destroy();

public:
    static std::atomic<uint32_t> connection_num;

private:
    static constexpr size_t MAX_BUFF_SIZE = 8192;
    // SSLSocket ssl_socket;
    boost::asio::io_context& io_context_;
    SSLSocket upstream_ssl_socket;
    // tcp::socket in_socket;
    tcp::socket downstream_socket;
    //
    std::string remote_host;
    std::string remote_port;
    //
    tcp::resolver resolver_;
    udp::resolver udp_resolver;
    udp::socket downstream_udp_socket;
    //
    std::vector<char> in_buf;
    std::vector<char> out_buf;
    State state_ { HANDSHAKE };
    TrojanReq trojanReq {};
    VRequest v_req {};
    std::string password;
    std::string upstream_udp_buff;
    bool vprotocol = false;
};