
#include "Coding.h"
#include <cstdint>
void Coding::EncodeFixed8(std::string& buf, uint8_t value)
{
    buf.append(1, value);
}
void Coding::EncodeFixed16(std::string& dst, uint16_t value)
{
    char buffer[2];
    // Recent clang and gcc optimize this to a single mov / str instruction.
    buffer[0] = static_cast<uint8_t>(value);
    buffer[1] = static_cast<uint8_t>(value >> 8);
    dst.append(buffer, 2);
}
void Coding::EncodeFixed32(std::string& dst, uint32_t value)
{
    char buffer[4];
    // Recent clang and gcc optimize this to a single mov / str instruction.
    buffer[0] = static_cast<uint8_t>(value);
    buffer[1] = static_cast<uint8_t>(value >> 8);
    buffer[2] = static_cast<uint8_t>(value >> 16);
    buffer[3] = static_cast<uint8_t>(value >> 24);
    dst.append(buffer, 4);
}
void Coding::EncodeStr(std::string& dst, std::string& src)
{
    dst.append(src);
}
 uint8_t Coding::DecodeFixed8(const char*ptr)
 {
     return static_cast<uint8_t>(ptr[0]);
 }
uint16_t Coding::DecodeFixed16(const char* ptr)
{

    return static_cast<uint16_t>(ptr[0]) | static_cast<uint16_t>(ptr[1])<<8;
}
uint32_t Coding::DecodeFixed32(const char* ptr)
{
    return static_cast<uint32_t>(ptr[0]) 
    | static_cast<uint32_t>(ptr[1])<<8
    |static_cast<uint32_t>(ptr[2])<<16
    |static_cast<uint32_t>(ptr[3])<<24;
}
std::string Coding::DecodeStr(const char* ptr, int len)
{
    return std::string(ptr,len);

}