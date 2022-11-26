#pragma once
#include"Session.h"

class WebsocketSession : public Session<websocket::stream<beast::ssl_stream<beast::tcp_stream>>>{

public:
    WebsocketSession(boost::asio::io_context&, boost::asio::ssl::context&);
    ~WebsocketSession(){
    }
    void start();

    void on_handshake(beast::error_code ec);
    void on_accept(beast::error_code ec);
    boost::asio::ip::tcp::socket& socket()
    {
        return beast::get_lowest_layer(upstream_socket).socket();
    }

    virtual void upstream_tcp_write(int direction, size_t len);
    virtual void upstream_udp_write(int direction, const std::string& packet);
    virtual void destroy();

};