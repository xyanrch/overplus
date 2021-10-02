#pragma once
#include <cstdint>
#include <functional>
#include <iostream>
#include <sstream>

enum Loglevel :uint8_t{
    L_DEBUG,
    L_NOTICE,
    L_ERROR_EXIT
};
void set_log_level(Loglevel level);
class logger {
public:
    logger(const char* func, int line, Loglevel level);
    logger(Loglevel level);

    ~logger();
    std::ostream& stream() { return impl.log_stream_; }
    static void setOutput(std::function<void(std::string&&)>&& outputFunc);
    static void setFlush(std::function<void()>&& fulsh);

private:
    struct Impl {
        Impl(const char* func, int line, Loglevel level);
        Impl(Loglevel level);
        ~Impl();
        std::ostringstream log_stream_;
        Loglevel level_;
    };
    Impl impl;
};

extern Loglevel log_level;
#define DEBUG_LOG             \
    if (log_level <= L_DEBUG) \
    logger(__func__, __LINE__, L_DEBUG).stream()
#define NOTICE_LOG             \
    if (log_level <= L_NOTICE) \
    logger(__func__, __LINE__, L_NOTICE).stream()
#define ERROR_LOG \
    logger(L_ERROR_EXIT).stream()
