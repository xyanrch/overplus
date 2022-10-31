#pragma once
#include <cstdint>
#include <functional>
#include <iostream>
#include <sstream>

enum Loglevel : uint8_t {
    L_DEBUG,
    L_NOTICE,
    L_ERROR_EXIT
};
enum Destination {
    D_STDOUT,
    D_FILE

};
class logger {
public:
    logger(const char* file, const char* func, int line, Loglevel level);
    logger(Loglevel level);

    ~logger();
    std::ostream& stream() { return impl.log_stream_; }
    static void setOutput(std::function<void(std::string&&)>&& outputFunc);
    static void setFlush(std::function<void()>&& fulsh);
    static void set_log_level(Loglevel level);
    static Loglevel get_log_level();
    static void set_log_destination(Destination dest);

private:
    struct Impl {
        Impl(const char* file, const char* func, int line, Loglevel level);
        Impl(Loglevel level);
        ~Impl();
        std::ostringstream log_stream_;
        Loglevel level_;
    };
    Impl impl;
};
void setLogLevel(Loglevel& level);

#define DEBUG_LOG                           \
    if (logger::get_log_level() <= L_DEBUG) \
    logger(__FILE__, __func__, __LINE__, L_DEBUG).stream()
#define NOTICE_LOG                           \
    if (logger::get_log_level() <= L_NOTICE) \
    logger(__FILE__, __func__, __LINE__, L_NOTICE).stream()
#define ERROR_LOG \
    logger(__FILE__, __func__, __LINE__,L_ERROR_EXIT).stream()
