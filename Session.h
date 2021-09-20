
#pragma once
#include "Log.h"
#include <boost/asio.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/core/noncopyable.hpp>
#include <boost/system/error_code.hpp>
#include <memory>
using boost::asio::ip::tcp;
class Session : public std::enable_shared_from_this<Session>
    , private boost::noncopyable {
    enum State {
        HANDSHAKE,
        FORWARD
    };

public:
    Session(boost::asio::io_context& context);

    void start();
    void do_read();
    boost::asio::ip::tcp::socket& socket();
    //void handle_sock5();
    void sock5_handshake();
    void sock5_read_packet(int);
    void write_sock5_hanshake_reply();
    void read_socks5_request();
    void do_resolve();
    void do_connect(tcp::resolver::iterator&);
    void write_socks5_response();
    void sock5_write_packet(int, size_t);

private:
    static constexpr size_t MAX_BUFF_SIZE = 8192;
    tcp::socket in_socket;
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
};