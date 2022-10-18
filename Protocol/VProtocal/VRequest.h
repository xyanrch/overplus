#pragma once

#include <cstdint>
#include <string>
#include<vector>
#include <cstring>
#include "Shared/Log.h"
class VRequest {
public:
    static constexpr const char* v_identifier = "v protocol";
    static constexpr int identifier_len = 10;
public:
    void stream(std::string& buf);
    bool unstream(const std::string& buf);
    static bool is_v_protocol(std::vector<char>&buf);
    

public:
    struct Header {
        std::string identifier;
        uint8_t version;
        std::uint32_t len;
    };
    Header header;
    std::string user_name;
    std::string password;
    //
    std::string address;
    uint16_t port;

    //packed buffer,  Don't stream/unstream this field
    std::string packed_buff;
};