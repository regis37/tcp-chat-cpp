#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>

#include <gtest/gtest.h>

#include <chrono>
#include <stdexcept>
#include <string>
#include <thread>

namespace test {

using namespace std::chrono_literals;
using Clock = std::chrono::steady_clock;

constexpr unsigned short kServerPort = 54000;

inline void ensureWinsock() {
    struct WinsockInit {
        WinsockInit() { WSADATA data; WSAStartup(MAKEWORD(2, 2), &data); }
        ~WinsockInit() { WSACleanup(); }
    };
    static WinsockInit init;
}

// Binding fails as soon as a server holds the port
inline bool serverPortIsFree() {
    ensureWinsock();
    SOCKET probe = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(kServerPort);
    addr.sin_addr.s_addr = INADDR_ANY;
    bool isFree = bind(probe, (sockaddr*)&addr, sizeof(addr)) == 0;
    closesocket(probe);
    return isFree;
}

// A raw TCP client that records everything the server sends
class TestClient {
public:
    TestClient() {
        ensureWinsock();
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(kServerPort);
        inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

        auto deadline = Clock::now() + 5s;
        while (true) {
            socket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (connect(socket_, (sockaddr*)&addr, sizeof(addr)) == 0) return;
            closesocket(socket_);
            socket_ = INVALID_SOCKET;
            if (Clock::now() > deadline) throw std::runtime_error("could not connect to server");
            std::this_thread::sleep_for(20ms);
        }
    }

    ~TestClient() {
        if (socket_ != INVALID_SOCKET) closesocket(socket_);
    }

    TestClient(const TestClient&) = delete;
    TestClient& operator=(const TestClient&) = delete;

    void send(const std::string& text) {
        ::send(socket_, text.c_str(), (int)text.size(), 0);
    }

    // Succeeds once `expected` has been received, consuming everything up to it
    ::testing::AssertionResult waitFor(const std::string& expected,
                                       std::chrono::milliseconds timeout = 2s) {
        auto deadline = Clock::now() + timeout;
        while (true) {
            auto pos = received_.find(expected);
            if (pos != std::string::npos) {
                received_.erase(0, pos + expected.size());
                return ::testing::AssertionSuccess() << "received \"" << expected << "\"";
            }
            if (!readSome(deadline)) {
                return ::testing::AssertionFailure()
                       << "expected \"" << expected << "\" within " << timeout.count()
                       << "ms, unread data: \"" << received_ << "\""
                       << (closed_ ? " (connection closed)" : "");
            }
        }
    }

    ::testing::AssertionResult waitForDisconnect(std::chrono::milliseconds timeout = 2s) {
        auto deadline = Clock::now() + timeout;
        while (!closed_ && readSome(deadline)) {}
        if (closed_) return ::testing::AssertionSuccess() << "connection closed";
        return ::testing::AssertionFailure() << "connection still open after " << timeout.count() << "ms";
    }

    ::testing::AssertionResult login(const std::string& username) {
        auto prompt = waitFor("Enter your username: ");
        if (!prompt) return prompt;
        send(username);
        return waitFor("Welcome " + username + "!");
    }

private:
    bool readSome(Clock::time_point deadline) {
        if (closed_) return false;
        auto remaining = std::chrono::duration_cast<std::chrono::microseconds>(deadline - Clock::now());
        if (remaining.count() <= 0) return false;

        fd_set readable;
        FD_ZERO(&readable);
        FD_SET(socket_, &readable);
        timeval tv{(long)(remaining.count() / 1000000), (long)(remaining.count() % 1000000)};
        if (select(0, &readable, nullptr, nullptr, &tv) <= 0) return false;

        char buffer[4096];
        int bytes = recv(socket_, buffer, sizeof(buffer), 0);
        if (bytes <= 0) {
            closed_ = true;
            return false;
        }
        received_.append(buffer, bytes);
        return true;
    }

    SOCKET socket_ = INVALID_SOCKET;
    std::string received_;
    bool closed_ = false;
};

} // namespace test
