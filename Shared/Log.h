#pragma once
#include <iostream>
#include <sstream>

enum Loglevel {
    L_DEBUG,
    L_NOTICE,
    L_ERROR_EXIT
};
class logger {
public:
    logger(std::ostream& os, const char* func, int line, Loglevel level);
    logger(std::ostream& os, Loglevel level);
    std::ostream& stream() { return impl.stream_; }

private:
    struct Impl {
        Impl(std::ostream& os, const char* func, int line, Loglevel level);
        Impl(std::ostream& os, Loglevel level);
        ~Impl();
        std::ostream& os_;
        std::stringstream stream_;
        Loglevel level_;
    };
    Impl impl;
};

extern Loglevel log_level;
#define DEBUG_LOG             \
    if (log_level <= L_DEBUG) \
    logger(std::cerr, __func__, __LINE__, L_DEBUG).stream()
#define NOTICE_LOG             \
    if (log_level <= L_NOTICE) \
    logger(std::cerr, __func__, __LINE__, L_NOTICE).stream()
#define ERROR_LOG \
    logger(std::cerr, L_ERROR_EXIT).stream()
