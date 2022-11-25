#pragma once

#include"Session.h"

class ServerSession :public Session<SSLSocket> {

public:
    ServerSession(boost::asio::io_context&, boost::asio::ssl::context&);

    ~ServerSession();

    void start();


    boost::asio::ip::tcp::socket& socket();

    virtual void upstream_tcp_write(int direction, size_t len);
    virtual void upstream_udp_write(int direction, const std::string& packet);
    virtual void destroy();

public:
    static std::atomic<uint32_t> connection_num;

};