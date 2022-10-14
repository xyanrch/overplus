#include "Log.h"
#include <ctime>
#include <ostream>
#include <thread>
#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_BLUE "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN "\x1b[36m"
#define ANSI_COLOR_RESET "\x1b[0m"

Loglevel log_level = L_NOTICE;
static Destination log_dest = D_STDOUT;

static std::function<void()> flush_ = []() {
    std::flush(std::cout);
};
static std::function<void(std::string&&)> output_ = [](std::string&& buf) {
    std::cout << buf;
};
const char* level_str[] {
    "[DEBUG] ",
    "[NOTICE] ",
    "[ERROR] "

};

static std::string get_format_time()
{
    time_t timep;
    time(&timep);
    char tmp[64];
    std::strftime(tmp, sizeof(tmp), "[%Y-%m-%d %H:%M:%S]", std::localtime(&timep));
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
void logger::set_log_level(Loglevel level)
{
    log_level = level;
}
Loglevel logger::get_log_level()
{
    return log_level;
}
void logger::set_log_destination(Destination dest)
{
    log_dest = dest;
}
logger::~logger()
{
    // auto buf = impl.log_stream_.str();
    impl.log_stream_ << "\n";
    output_(impl.log_stream_.str());
    if (impl.level_ == L_ERROR_EXIT) {
        flush_();
        // abort();
    }
}
logger::logger(const char* file, const char* func, int line, Loglevel level)
    : impl(file, func, line, level)
{
}
logger::logger(Loglevel level)
    : impl(level)
{
}
logger::Impl::Impl(const char* file, const char* func, int line, Loglevel level)
{
    if (log_dest == D_STDOUT) {
        log_stream_ << get_format_time() << ANSI_COLOR_GREEN << level_str[level] << ANSI_COLOR_RESET << file << " " << func << ": line: " << line << " ";
    } else {
        log_stream_ << get_format_time() << level_str[level] << file << " " << func << ": line: " << line << " ";
    }
    log_stream_ << "[" << std::this_thread::get_id() << "] ";

    level_ = level;
}

logger::Impl::Impl(Loglevel level)
{
    if (log_dest == D_STDOUT) {
        log_stream_ << get_format_time() << ANSI_COLOR_RED << level_str[level] << ANSI_COLOR_RESET << " ";

    } else {
        log_stream_ << get_format_time() << level_str[level] << " ";
    }
    log_stream_ << "[" << std::this_thread::get_id() << "] ";
    level_ = level;
}
logger::Impl::~Impl()
{
    // log_stream_ << "\n";
}
