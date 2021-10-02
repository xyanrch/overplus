#include "LogFile.h"
#include <assert.h>
#include <cstdio>
#include <mutex>

/*  FILE* fp_;
    char buffer_[64 * 1024];
    off_t writtenBytes_;
    */
AppendFile::AppendFile(const char* filename)
{
    fp_ = fopen(filename, "ae");
    assert(fp_);
}

AppendFile::~AppendFile()
{
    // if (fp_) {
    ::fflush(fp_);
    ::fclose(fp_);
    // }
}

void AppendFile::append(const char* logline, size_t len)
{
    writtenBytes_ += len;
    ::fwrite(logline, len, 1, fp_);
}

void AppendFile::flush()
{
    ::fflush(fp_);
}

LogFile::LogFile(const std::string& basename,
    off_t rollSize,
    bool threadSafe,
    int flushInterval,
    int checkEveryN)
    : basename_(basename)
    , rollSize_(rollSize)
    , flushInterval_(flushInterval)
    , checkEveryN_(checkEveryN)
    , count_(0)
    , muti_threads(threadSafe)
    //mutex_(threadSafe ? new MutexLock : NULL),
    , startOfPeriod_(0)
    , lastRoll_(0)
    , lastFlush_(0)
{
    assert(basename.find('/') == std::string::npos);
    rollFile();
}

void LogFile::append(std::string&& buf)
{
    if (muti_threads) {
        std::lock_guard<std::mutex> lock_guard(mutex_);
        append_unlocked(std::move(buf));
    }
    // else
    {
        append_unlocked(std::move(buf));
    }
}

void LogFile::flush()
{
    if (muti_threads) {
        std::lock_guard<std::mutex> lock_guard(mutex_);
        file_->flush();
    } else {
        file_->flush();
    }
}

void LogFile::append_unlocked(std::string&& buf)
{
    file_->append(buf.data(), buf.length());

    if (file_->writtenBytes() > rollSize_) {
        rollFile();
    } else {
        ++count_;
        if (count_ >= checkEveryN_) {
            count_ = 0;
            time_t now = ::time(NULL);
            if (now - lastFlush_ > flushInterval_) {
                lastFlush_ = now;
                file_->flush();
            }
        }
    }
}

bool LogFile::rollFile()
{
    time_t now = 0;
    std::string filename = getLogFileName(basename_, &now);
    time_t start = now / kRollPerSeconds_ * kRollPerSeconds_;

    if (now > lastRoll_) {
        lastRoll_ = now;
        lastFlush_ = now;
        startOfPeriod_ = start;
        file_.reset(new AppendFile(filename.c_str()));
        return true;
    }
    return false;
}

std::string LogFile::getLogFileName(const std::string& basename, time_t* now)
{
    std::string filename;
    filename.reserve(basename.size() + 64);
    filename = basename;

    char timebuf[32];
    struct tm tm;
    *now = time(NULL);
    gmtime_r(now, &tm); // FIXME: localtime_r ?
    strftime(timebuf, sizeof timebuf, ".%Y%m%d-%H%M%S", &tm);
    filename += timebuf;

    /*filename += ProcessInfo::hostname();

    char pidbuf[32];
    snprintf(pidbuf, sizeof pidbuf, ".%d", ProcessInfo::pid());
    filename += pidbuf;
*/
    filename += ".log";

    return filename;
}