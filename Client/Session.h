
#pragma once
#include "Shared/Log.h"
#include <Protocol/socks5/socks5.h>
#include <boost/asio.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/core/noncopyable.hpp>
#include <boost/system/error_code.hpp>
#include <memory>
#include <vector>
using boost::asio::ip::tcp;
class Session : public std::enable_shared_from_this<Session>
    , private boost::noncopyable {
    enum State {
        HANDSHAKE,
        FORWARD
    };

public:
    Session(boost::asio::io_context& context, boost::asio::ssl::context& ssl);

    void start();
    void do_read();
    boost::asio::ip::tcp::socket& socket();
    // void handle_sock5();
    void sock5_handshake();
    void read_packet(int);
    void write_sock5_hanshake_reply(AuthReq& req);
    void read_socks5_request();
    void do_resolve();
    void do_connect(tcp::resolver::iterator&);
    void write_socks5_response();
    void write_packet(int, size_t);
    void do_sent_v_req();
    void do_ssl_handshake();
    void destroy();

private:
    static constexpr size_t MAX_BUFF_SIZE = 8192;
    boost::asio::io_context& context_;
    tcp::socket in_socket;

    //
    std::string remote_host;
    std::string remote_port;
    //
    tcp::resolver resolver_;
    //
    std::vector<char> in_buf;
    std::vector<char> out_buf;
    std::string message_buf;
    Request socks5_req;
    State state_ { HANDSHAKE };
    boost::asio::ssl::context& ssl_ctx;
    boost::asio::ssl::stream<boost::asio::ip::tcp::socket> out_socket;
};