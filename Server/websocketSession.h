#pragma once
#include"Session.h"
#include <Protocol/TrojanReq.h>
#include <Protocol/VProtocal/VRequest.h>
#include <boost/asio.hpp>
#include<boost/noncopyable.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
namespace ssl = boost::asio::ssl;       // from <boost/asio/ssl.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>
class WebsocketSession : public std::enable_shared_from_this<WebsocketSession>
    , private boost::noncopyable {

public:
    WebsocketSession(boost::asio::io_context&, boost::asio::ssl::context&);
    void start();

    void on_handshake(beast::error_code ec);
    void on_accept(beast::error_code ec);
    void do_resolve();
    void do_connect(tcp::resolver::iterator& it);
    void async_bidirectional_write(int, size_t);
    void async_bidirectional_read(int direction);
    void destroy();

    void handle_trojan_udp_proxy();
    void udp_upstream_read();
    void udp_async_bidirectional_read(int direction);
    void udp_async_bidirectional_write(int, const std::string&, boost::asio::ip::udp::resolver::iterator);
    boost::asio::ip::tcp::socket& socket()
    {
        return beast::get_lowest_layer(ws_).socket();
    }

private:
    static constexpr size_t MAX_BUFF_SIZE = 8192;
    TrojanReq trojanReq {};
    VRequest v_req {};
    std::string password;
    std::string upstream_udp_buff;
    bool vprotocol = false;

    //
    boost::asio::io_context& io_context_;
    // tcp::socket in_socket;
    tcp::socket downstream_socket;
    tcp::resolver resolver_;
    //
    std::string remote_host;
    std::string remote_port;
    //


    //
    std::vector<char> in_buf;
    std::vector<char> out_buf;
    //
    websocket::stream<beast::ssl_stream<beast::tcp_stream>> ws_;
    beast::flat_buffer buffer_;
    udp::resolver udp_resolver;
    udp::socket downstream_udp_socket;
};