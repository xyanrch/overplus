#pragma once

#include <cstdint>
#include <string>
class VRequest {
public:
    enum Command:uint8_t {
        CONNECT = 0,
        UDP_ASSOCIATE = 1
    };

    enum AddressType:uint8_t {
        IPv4 = 0,
        DOMAINNAME = 1,
        IPv6 = 2
    };

    void stream(std::string& buf);
      bool unstream(const std::string& buf);
    

private:
    struct Header {
        uint8_t version;
        std::uint32_t len;
    };
    Header header;
    std::string password;
    //
    Command command;
    //
    AddressType address_type;
    std::string address;
    uint16_t port;
};