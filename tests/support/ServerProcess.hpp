#pragma once

// Keeps windows.h from pulling in the old winsock.h, which clashes with winsock2.h
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <filesystem>
#include <stdexcept>
#include <string>

namespace test {

// Runs server.exe in its own working directory and kills it on destruction
class ServerProcess {
public:
    ServerProcess(const std::string& exePath, const std::filesystem::path& workDir) {
        STARTUPINFOA startup{};
        startup.cb = sizeof(startup);
        std::string commandLine = "\"" + exePath + "\"";
        std::string dir = workDir.string();

        if (!CreateProcessA(exePath.c_str(), commandLine.data(), nullptr, nullptr, FALSE,
                            CREATE_NO_WINDOW, nullptr, dir.c_str(), &startup, &process_)) {
            throw std::runtime_error("could not start " + exePath);
        }
    }

    ~ServerProcess() {
        TerminateProcess(process_.hProcess, 0);
        WaitForSingleObject(process_.hProcess, 5000);
        CloseHandle(process_.hThread);
        CloseHandle(process_.hProcess);
    }

    ServerProcess(const ServerProcess&) = delete;
    ServerProcess& operator=(const ServerProcess&) = delete;

    bool isRunning() const {
        return WaitForSingleObject(process_.hProcess, 0) == WAIT_TIMEOUT;
    }

private:
    PROCESS_INFORMATION process_{};
};

} // namespace test
