#pragma once
#include "Shared/Log.h"
#include <Protocal/TrojanReq.h>
#include <boost/asio.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/core/noncopyable.hpp>
#include <boost/system/error_code.hpp>
#include <memory>
using SSLSocket = boost::asio::ssl::stream<boost::asio::ip::tcp::socket>;
using boost::asio::ip::tcp;
class ServerSession : public std::enable_shared_from_this<ServerSession>
    , private boost::noncopyable {
    enum State {
        HANDSHAKE,
        FORWARD
    };

public:
    ServerSession(boost::asio::io_context&, boost::asio::ssl::context&);

    void start();
    void do_read();
    boost::asio::ip::tcp::socket& socket();
    //void handle_sock5();
    void in_async_read(int direction);
    void handle_trojan_handshake();
    void do_resolve();
    void do_connect(tcp::resolver::iterator&);
    void out_async_write(int, size_t);

    void destroy();

private:
    static constexpr size_t MAX_BUFF_SIZE = 8192;
    //SSLSocket ssl_socket;
    boost::asio::io_context& io_context_;
    SSLSocket in_ssl_socket;
    //tcp::socket in_socket;
    tcp::socket out_socket;
    //
    std::string remote_host;
    std::string remote_port;
    //
    tcp::resolver resolver_;
    //
    std::vector<char> in_buf;
    std::vector<char> out_buf;
    State state_ { HANDSHAKE };
    TrojanReq req {};
    //unsigned char temp[4096] {};
};