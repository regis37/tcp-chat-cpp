#pragma once

#include <mutex>
#include <string>

// Appends timestamped lines to a log file; safe to call from several threads
class Logger {
public:
    explicit Logger(std::string path);

    void log(const std::string& message);

private:
    std::string path_;
    std::mutex mutex_;
};
