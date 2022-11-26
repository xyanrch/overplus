#pragma once

#include"Session.h"

class TlsSession :public Session<SSLSocket> {

public:
    TlsSession(boost::asio::io_context&, boost::asio::ssl::context&);

    ~TlsSession();

    void start();


    boost::asio::ip::tcp::socket& socket();

    virtual void upstream_tcp_write(int direction, size_t len);
    virtual void upstream_udp_write(int direction, const std::string& packet);
    virtual void destroy();

};