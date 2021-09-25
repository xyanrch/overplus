#pragma once
#include <boost/asio.hpp>
#include <string>
class SOCKS5Address {
public:
    enum AddressType {
        IPv4 = 1,
        DOMAINNAME = 3,
        IPv6 = 4
    } address_type;
    std::string address;
    uint16_t port;
    bool parse(const std::string& data, size_t& address_len);
    static std::string generate(const boost::asio::ip::udp::endpoint& endpoint);
};
class TrojanReq {
public:
    enum Command {
        CONNECT = 1,
        UDP_ASSOCIATE = 3
    } command;

    //
    std::string password;
    SOCKS5Address address;
    std::string payload;
    int parse(const std::string& data);
    static std::string generate(const std::string& password, const std::string& domainname, uint16_t port, bool tcp);
};