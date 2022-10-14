#pragma once

#include <cstdint>
#include <string>
class VRequest {
public:
    void stream(std::string& buf);
    bool unstream(const std::string& buf);
    

public:
    struct Header {
        std::string protocal = "v protocal";
        uint8_t version;
        std::uint32_t len;
    };
    Header header;
    std::string user_name;
    std::string password;
    //
    std::string address;
    uint16_t port;
};