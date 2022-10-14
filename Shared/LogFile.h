
#pragma once
#include <memory>
#include <mutex>
#include <string>
class AppendFile {
public:
    explicit AppendFile(const char* filename);

    ~AppendFile();

    void append(const char* logline, size_t len);

    void flush();

    off_t writtenBytes() const { return writtenBytes_; }

private:
    FILE* fp_ { nullptr };
    off_t writtenBytes_ { 0 };
};
class LogFile {
public:
    LogFile(const std::string& basename,
        off_t rollSize,
        bool threadSafe = true,
        int flushInterval = 3,
        int checkEveryN = 1024);
    ~LogFile() = default;

    void append(std::string&& buf);
    void flush();
    bool rollFile();

private:
    void append_unlocked(std::string&&);

    static std::string getLogFileName(const std::string& basename, time_t* now);

    const std::string basename_;
    const off_t rollSize_;
    const int flushInterval_;
    const int checkEveryN_;

    int count_;
    bool muti_threads;
    std::mutex mutex_;
    time_t startOfPeriod_;
    time_t lastRoll_;
    time_t lastFlush_;
    std::unique_ptr<AppendFile> file_;

    const static int kRollPerSeconds_ = 60 * 60 * 24;
};