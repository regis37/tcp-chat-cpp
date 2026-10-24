#pragma once

// winsock2.h must come before windows.h, and iphlpapi.h after both
#include <winsock2.h>
#include <windows.h>
#include <iphlpapi.h>

#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>

namespace test {

// Asks Windows which process is listening on `port` (0 if none).
// Unlike trying to bind the port ourselves, this can't steal it from a server that is starting.
inline DWORD listeningProcessId(unsigned short port) {
    std::vector<char> buffer;
    ULONG size = 0;
    DWORD result;
    do {
        buffer.resize(size);
        result = GetExtendedTcpTable(buffer.data(), &size, FALSE, AF_INET,
                                     TCP_TABLE_OWNER_PID_LISTENER, 0);
    } while (result == ERROR_INSUFFICIENT_BUFFER);
    if (result != NO_ERROR) return 0;

    auto* table = reinterpret_cast<MIB_TCPTABLE_OWNER_PID*>(buffer.data());
    for (DWORD i = 0; i < table->dwNumEntries; ++i) {
        if (ntohs((u_short)table->table[i].dwLocalPort) == port) {
            return table->table[i].dwOwningPid;
        }
    }
    return 0;
}

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

    DWORD pid() const { return process_.dwProcessId; }

private:
    PROCESS_INFORMATION process_{};
};

} // namespace test
