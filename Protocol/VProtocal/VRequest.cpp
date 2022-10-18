
#include "VRequest.h"
#include "Shared/Coding.h"
#include "Shared/Log.h"
#include <cstdint>

#ifdef _WIN32
#    include <WinSock2.h>
#else

#    include <netinet/in.h>

#endif

void VRequest::stream(std::string& buf)
{

    header.len = 5 + identifier_len + password.length()+4 + user_name.length()+4 + address.length()+4 + sizeof(port);
    Coding::EncodeCstr(buf, v_identifier);
    Coding::EncodeFixed8(buf, header.version);
    Coding::EncodeFixed32(buf, htonl(header.len));

    //
    Coding::EncodeFixed32(buf, htonl(user_name.length()));
    Coding::EncodeStr(buf, user_name);
    Coding::EncodeFixed32(buf, htonl(password.length()));
    Coding::EncodeStr(buf, password);
    //
    Coding::EncodeFixed32(buf, htonl(address.length()));
    Coding::EncodeStr(buf, address);
    //
    Coding::EncodeFixed16(buf, htons(port));
    DEBUG_LOG<<"header.len=" << header.len<<  " dump buf:" << buf;
}

bool VRequest::unstream(const std::string& buf)
{
    if (buf.length() < sizeof(Header)) {
        NOTICE_LOG << "Buf len:" << buf.length() << " head size:" << sizeof(Header);
        return false;
    }
    // int pos = 0;
    char* ptr = const_cast<char*>(buf.data());
    // skip identifier
    ptr += identifier_len;
    this->header.version = *ptr;
    ptr++;
    this->header.len = ntohl(Coding::DecodeFixed32(ptr));
    if (this->header.len > buf.length()) {
        NOTICE_LOG << "header.len:" << this->header.len << "  buf length:" << buf.length();
        return false;
    }
    ptr += 4;
    uint32_t len = ntohl(Coding::DecodeFixed32(ptr));
    ptr += 4;
    this->user_name = Coding::DecodeStr(ptr, len);
    ptr += len;
    len = ntohl(Coding::DecodeFixed32(ptr));
    ptr += 4;
    this->password = Coding::DecodeStr(ptr, len);
    ptr += len;
    len = ntohl(Coding::DecodeFixed32(ptr));
    ptr += 4;
    this->address = Coding::DecodeStr(ptr, len);
    ptr += len;
    this->port = ntohs(Coding::DecodeFixed16(ptr));
    packed_buff.assign(ptr+2,buf.size()-header.len);
    return true;
}

bool VRequest::is_v_protocol(std::vector<char>& buf)
{
    DEBUG_LOG << "Incoming identifier:" << std::string(buf.data(), identifier_len);
    return memcmp(v_identifier, buf.data(), identifier_len) == 0;
}
