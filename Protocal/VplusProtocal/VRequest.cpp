
#include "VRequest.h"
#include "Coding.h"
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
    Encode::EncodeFixed8(buf,header.version);
    Encode::EncodeFixed32(buf, header.len);
    //
    Encode::EncodeFixed32(buf, password.length());
    Encode::EncodeStr(buf, password);
    //
    Encode::EncodeFixed8(buf, command);
    Encode::EncodeFixed8(buf, address_type);
    //
    Encode::EncodeFixed32(buf, address.length());
    Encode::EncodeStr(buf, address);
    //
    Encode::EncodeFixed16(buf, port);
}
void VRequest::unstream(const std::string& buf)
{
}
