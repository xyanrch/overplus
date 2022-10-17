//
// socks5 specification
// https://www.rfc-editor.org/rfc/rfc1928

#pragma once
#include "Shared/Coding.h"

#include <Shared/Log.h>
#include <boost/asio.hpp>
#include <cstddef>
#include <cstdint>
#include <ostream>
#include <string>
#include <vector>
enum AuthMethod : uint8_t {
    NO_AUTHENTICATION,    // 0X00
    GSSAPI,               // 0X01
    PASSWORD,             // 0X02
    IANA_ASSIGNED,        // 0X03-0X7E
    RESERVED,             // 0X80-0XFE
    NO_ACCEPTABLE_METHODS // 0XFF

};
std::string method_to_string(AuthMethod method);

class AuthReq {
public:
    void stream(std::string& buf)
    {
    }
    bool unstream(std::vector<char>& buf);

public:
    uint8_t version;
    uint8_t nmethod;
    std::vector<AuthMethod> methods;
};
std::ostream& operator<<(std::ostream& os, AuthReq& req);

class AuthRes {
public:
    void stream(std::string& buf)
    {
        Coding::EncodeFixed8(buf, version);
        Coding::EncodeFixed8(buf, method);
    }

public:
    uint8_t version;
    AuthMethod method;
};
/* +----+-----+-------+------+----------+----------+
       |VER | CMD |  RSV  | ATYP | DST.ADDR | DST.PORT |
       +----+-----+-------+------+----------+----------+
       | 1  |  1  | X'00' |  1   | Variable |    2     |
       +----+-----+-------+------+----------+----------+
*/
enum ADDRTYPE : uint8_t {
    V4 = 0x01,
    DOMAINNAME = 0x03,
    V6 = 0x04
};
class Request {
public:
    enum CMD : uint8_t {
        CONNECT = 0x01,
        BIND = 0x02,
        UDP_ASSOCIATE = 0x03

    };

    static std::string command_to_string(CMD cmd);

public:
    bool unsteam(std::vector<char>& buf, int length);

public:
    uint8_t version;
    CMD cmd;
    uint8_t reserved;
    uint8_t addrType;
    std::string remote_host;
    uint16_t remote_port;
};
std::ostream& operator<<(std::ostream& os, Request& req);

class Reply {
public:
    void stream(std::string& buf)
    {
        Coding::EncodeFixed8(buf, version);
        Coding::EncodeFixed8(buf, repResult);
        Coding::EncodeFixed8(buf, reserved);

        Coding::EncodeFixed8(buf, static_cast<uint8_t>(addrtype));
        Coding::EncodeFixed32(buf, htonl(realRemoteIP));
        Coding::EncodeFixed16(buf, htons(realRemotePort));
    }

public:
    uint8_t version;
    uint8_t repResult;
    uint8_t reserved;
    ADDRTYPE addrtype;
    uint32_t realRemoteIP;
    uint16_t realRemotePort;
};
