#pragma once

#include <cstdint>
#include <string>
class Coding {
public:
    static void EncodeFixed8(std::string& buf, uint8_t value);

    static void EncodeFixed16(std::string& dst, uint16_t value);

    static void EncodeFixed32(std::string& dst, uint32_t value);

    static void EncodeStr(std::string& dst, std::string& src);
    static  void EncodeCstr(std::string& dst, const char* src);

    //
    static uint8_t DecodeFixed8(const char*ptr);
    static uint16_t DecodeFixed16(const char*ptr);
    static uint32_t DecodeFixed32(const char*ptr);
    static std::string DecodeStr(const char*ptr,int len);
};
