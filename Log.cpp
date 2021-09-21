#include "Log.h"
#include <ctime>
#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_BLUE "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN "\x1b[36m"
#define ANSI_COLOR_RESET "\x1b[0m"

Loglevel log_level = L_DEBUG;
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

logger::logger(std::ostream& os, const char* func, int line, Loglevel level)
    : impl(os, func, line, level)
{
}
logger::logger(std::ostream& os, Loglevel level)
    : impl(os, level)
{
}
logger::Impl::Impl(std::ostream& os, const char* func, int line, Loglevel level)
    : os_(os)
{
    stream_ << get_format_time() << ANSI_COLOR_GREEN << levelToString(level) << ANSI_COLOR_RESET << func << ": line: " << line << " ";
    level_ = level;
}

logger::Impl::Impl(std::ostream& os, Loglevel level)
    : os_(os)
{
    stream_ << get_format_time() << ANSI_COLOR_RED << levelToString(level) << ANSI_COLOR_RESET << " ";
    level_ = level;
}
logger::Impl::~Impl()
{
    os_ << stream_.str() << std::endl;
}
