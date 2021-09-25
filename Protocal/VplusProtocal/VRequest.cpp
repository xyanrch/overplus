
#include "VRequest.h"
#include "Coding.h"
#include <cstdint>
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
    //header.len = sizeof(Header)+password.length()+sizeof(Command)+sizeof(AddressType)+address.length()+sizeof(port);
    Coding::EncodeFixed8(buf,header.version);
    Coding::EncodeFixed32(buf, header.len);
    //
    Coding::EncodeFixed32(buf, password.length());
    Coding::EncodeStr(buf, password);
    //
    Coding::EncodeFixed8(buf, command);
    Coding::EncodeFixed8(buf, address_type);
    //
    Coding::EncodeFixed32(buf, address.length());
    Coding::EncodeStr(buf, address);
    //
    Coding::EncodeFixed16(buf, port);
}
bool VRequest::unstream(const std::string& buf)
{
    if(buf.length()<sizeof(Header))
    {

        return false;
    }
    //int pos = 0;
    char* ptr  = const_cast<char*>(buf.data());
    this->header.version = ptr[0];
    ptr++;
    this->header.len = Coding::DecodeFixed32(ptr);
    if(header.len>buf.length())
    {
        return false;
    }
    ptr +=4;
    uint32_t len = Coding::DecodeFixed32(ptr);
    ptr+=4;
    this->password = Coding::DecodeStr(ptr, len);
    ptr +=len;
    this->command = static_cast<Command>(*ptr);
    ptr++;
    this->address_type = static_cast<AddressType>(*ptr);
    ptr++;
    len = Coding::DecodeFixed32(ptr);
    ptr+=4;
    this->address = Coding::DecodeStr(ptr, len);
    ptr+=len;
    this->port = Coding::DecodeFixed16(ptr);
    return true;
}
