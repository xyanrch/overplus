
#include "VRequest.h"
#include "../Coding.h"
#include <cstdint>
#ifdef _WIN32
#include<WinSock2.h>
#else
#include <netinet/in.h>
#endif

void VRequest::stream(std::string& buf)
{
    header.len = 5 + identifier_len + password.length() + user_name.length() + address.length() + sizeof(port);
    Coding::EncodeCstr(buf, v_identifier);
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
    //skip identifier
    ptr += identifier_len;
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
 bool VRequest::is_v_protocol(std::vector<char>&buf)
{
    DEBUG_LOG<<"Incoming identifier:"<<std::string(buf.data(),identifier_len);
    return memcmp(v_identifier, buf.data(),identifier_len)==0;
}

