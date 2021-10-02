#include "Log.h"
#include <ctime>
#include <ostream>
#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_BLUE "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN "\x1b[36m"
#define ANSI_COLOR_RESET "\x1b[0m"

Loglevel log_level = L_NOTICE;
static std::function<void()> flush_ = []() {
    std::flush(std::cout);
};
static std::function<void(std::string&&)> output_ = [](std::string&& buf) {
    std::cout << buf;
};

static std::string levelToString(Loglevel l)
{
    switch (l) {
    case L_DEBUG:
        return "[DEBUG] ";
    case L_NOTICE:
        return "[NOTICE] ";
    case L_ERROR_EXIT:
        return "[ERROR] ";
    }
    return "[NA]";
};

static std::string get_format_time()
{
    time_t timep;
    time(&timep);
    char tmp[64];
    std::strftime(tmp, sizeof(tmp), "[%Y-%m-%d %h:%M:%S]", std::localtime(&timep));
    return tmp;
}
void logger::setOutput(std::function<void(std::string&&)>&& outputFunc)
{
    output_ = outputFunc;
}
void logger::setFlush(std::function<void()>&& flush)
{
    flush_ = flush;
}
logger::~logger()
{
    //auto buf = impl.log_stream_.str();
    output_(impl.log_stream_.str());
    if (impl.level_ == L_ERROR_EXIT) {
        flush_();
        //abort();
    }
}
logger::logger(const char* func, int line, Loglevel level)
    : impl(func, line, level)
{
}
logger::logger(Loglevel level)
    : impl(level)
{
}
logger::Impl::Impl(const char* func, int line, Loglevel level)
{
    log_stream_ << get_format_time() << ANSI_COLOR_GREEN << levelToString(level) << ANSI_COLOR_RESET << func << ": line: " << line << " ";
    level_ = level;
}

logger::Impl::Impl(Loglevel level)
{
    log_stream_ << get_format_time() << ANSI_COLOR_RED << levelToString(level) << ANSI_COLOR_RESET << " ";
    level_ = level;
}
logger::Impl::~Impl()
{
    log_stream_ << "\n";
}
