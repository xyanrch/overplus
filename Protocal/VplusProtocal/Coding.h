#pragma once

#include <cstdint>
#include <string>
class Encode {
public:
    static void EncodeFixed8(std::string& buf, uint8_t value);

    static void EncodeFixed16(std::string& dst, uint16_t value);

    static void EncodeFixed32(std::string& dst, uint32_t value);

    static void EncodeStr(std::string& dst, std::string& src);
};
class Decode {
public:
};