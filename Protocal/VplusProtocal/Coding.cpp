
#include "Coding.h"
void Encode::Fixed8(std::string& buf, uint8_t value)
{
    buf.append(1, value);
}
void Encode::EncodeFixed16(std::string& dst, uint16_t value)
{
    char buffer[2];
    // Recent clang and gcc optimize this to a single mov / str instruction.
    buffer[0] = static_cast<uint8_t>(value);
    buffer[1] = static_cast<uint8_t>(value >> 8);
    dst.append(buffer, 2);
}
void Encode::EncodeFixed32(std::string& dst, uint32_t value)
{
    char buffer[4];
    // Recent clang and gcc optimize this to a single mov / str instruction.
    buffer[0] = static_cast<uint8_t>(value);
    buffer[1] = static_cast<uint8_t>(value >> 8);
    buffer[2] = static_cast<uint8_t>(value >> 16);
    buffer[3] = static_cast<uint8_t>(value >> 24);
    dst.append(buffer, 4);
}
void Encode::EncodeStr(std::string& dst, std::string& src)
{
    dst.append(src);
}