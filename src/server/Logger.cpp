#include "Logger.hpp"

#include <ctime>
#include <fstream>
#include <utility>

namespace {

// Example: [2026-04-14 22:30:15]
std::string timestamp() {
    std::time_t now = std::time(nullptr);
    char buf[20];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
    return std::string("[") + buf + "]";
}

} // namespace

Logger::Logger(std::string path) : path_(std::move(path)) {}

void Logger::log(const std::string& message) {
    // One writer at a time, otherwise two lines can land at the same offset and overwrite each other.
    // Also covers std::localtime, which returns a buffer shared by all threads.
    std::lock_guard<std::mutex> lock(mutex_);
    std::ofstream file(path_, std::ios::app);
    if (file.is_open()) {
        file << timestamp() << " " << message << "\n";
    }
}
