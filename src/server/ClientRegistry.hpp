#pragma once

#include <winsock2.h>

#include <mutex>
#include <string>
#include <vector>

// The list of connected clients. Every access goes through the mutex,
// so no caller can touch the list without locking it.
//
// A client must be removed before its socket is closed: then any socket
// still in the list is open, and sending to it while holding the lock is safe.
class ClientRegistry {
public:
    void add(SOCKET socket);
    void setUsername(SOCKET socket, const std::string& username);
    void remove(SOCKET socket);

    void broadcast(const std::string& message, SOCKET except);
    // Returns false if nobody has that username
    bool sendTo(const std::string& username, const std::string& message);

    std::vector<std::string> usernames();

private:
    struct Client {
        SOCKET socket;
        std::string username;
    };

    std::vector<Client> clients_;
    std::mutex mutex_;
};
