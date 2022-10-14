
#include "VRequest.h"
#include "../Coding.h"
#include <cstdint>
#ifdef _WIN32
#include<WinSock2.h>
#else
#include <netinet/in.h>
#endif
/*
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
    */
void VRequest::stream(std::string& buf)
{
    header.len = 5 + header.protocal.length() + password.length() + user_name.length() + address.length() + sizeof(port);
    Coding::EncodeStr(buf, header.protocal);
    Coding::EncodeFixed8(buf, header.version);
    Coding::EncodeFixed32(buf, header.len);
    //
    Coding::EncodeFixed32(buf, user_name.length());
    Coding::EncodeStr(buf, user_name);
    Coding::EncodeFixed32(buf, password.length());
    Coding::EncodeStr(buf, password);
    //
    Coding::EncodeFixed32(buf, address.length());
    Coding::EncodeStr(buf, address);
    //
    Coding::EncodeFixed16(buf, htons(port));
}
bool VRequest::unstream(const std::string& buf)
{
    if (buf.length() < sizeof(Header)) {

        return false;
    }
    // int pos = 0;
    char* ptr = const_cast<char*>(buf.data());
    auto id = std::string(ptr, 10);
    ptr += 10;
    this->header.version = *ptr;
    ptr++;
    this->header.len = Coding::DecodeFixed32(ptr);
    if (header.len > buf.length()) {
        return false;
    }
    ptr += 4;
    uint32_t len = Coding::DecodeFixed32(ptr);
    ptr += 4;
    this->user_name = Coding::DecodeStr(ptr, len);
    ptr += len;
    len = Coding::DecodeFixed32(ptr);
    ptr += 4;
    this->password = Coding::DecodeStr(ptr, len);
    ptr += len;
    len = Coding::DecodeFixed32(ptr);
    ptr += 4;
    this->address = Coding::DecodeStr(ptr, len);
    ptr += len;
    this->port = ntohs(Coding::DecodeFixed16(ptr));
    return true;
}
