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

using SSLSocket = boost::asio::ssl::stream<boost::asio::ip::tcp::socket>;
using boost::asio::ip::tcp;
using boost::asio::ip::udp;
template<class T>
class Session :public std::enable_shared_from_this<Session<T>>
    , private boost::noncopyable
{
public:
    enum State {
        HANDSHAKE,
        FORWARD,
        DESTROY
    };
    enum Type{
        WEBSOCKET,
        TCP
    };
public:
    Session(boost::asio::io_context&, boost::asio::ssl::context&,Session::Type type);

    virtual ~Session(){}
   //  void start() ;
   //  boost::asio::ip::tcp::socket& socket();


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

protected:
    static constexpr size_t MAX_BUFF_SIZE = 8192;
    // SSLSocket ssl_socket;
    boost::asio::io_context& io_context_;
    T upstream_ssl_socket;
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
    Type session_type;

};
#include "Session.cpp"