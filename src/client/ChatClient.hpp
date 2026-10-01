#pragma once

#include <winsock2.h>

#include <atomic>

// Connects to the server, prints what it receives and sends what the user types
class ChatClient {
public:
    // Throws if the server can't be reached
    ChatClient(const char* address, unsigned short port);
    ~ChatClient();

    ChatClient(const ChatClient&) = delete;
    ChatClient& operator=(const ChatClient&) = delete;

    // Returns when the user types "quit" or after the server has disconnected
    void run();

private:
    void receiveLoop();

    SOCKET socket_ = INVALID_SOCKET;
    // Shared by the input loop and the receiver thread
    std::atomic<bool> running_{true};
};
