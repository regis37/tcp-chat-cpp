#pragma once

#include <winsock2.h>

#include <stdexcept>

// Starts Winsock when created and stops it when destroyed (RAII),
// so WSACleanup can't be forgotten on an early return or an exception
class WinsockSession {
public:
    WinsockSession() {
        WSADATA data;
        if (WSAStartup(MAKEWORD(2, 2), &data) != 0) {
            throw std::runtime_error("WSAStartup failed");
        }
    }

    ~WinsockSession() { WSACleanup(); }

    WinsockSession(const WinsockSession&) = delete;
    WinsockSession& operator=(const WinsockSession&) = delete;
};
