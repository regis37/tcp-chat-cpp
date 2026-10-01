#include "ClientRegistry.hpp"

#include "common/SocketIO.hpp"

#include <algorithm>

void ClientRegistry::add(SOCKET socket) {
    std::lock_guard<std::mutex> lock(mutex_);
    clients_.push_back({socket, ""});
}

void ClientRegistry::setUsername(SOCKET socket, const std::string& username) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (Client& c : clients_) {
        if (c.socket == socket) {
            c.username = username;
            return;
        }
    }
}

void ClientRegistry::remove(SOCKET socket) {
    std::lock_guard<std::mutex> lock(mutex_);
    clients_.erase(
        std::remove_if(clients_.begin(), clients_.end(),
            [socket](const Client& c) { return c.socket == socket; }),
        clients_.end()
    );
}

void ClientRegistry::broadcast(const std::string& message, SOCKET except) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (const Client& c : clients_) {
        if (c.socket != except) {
            sendText(c.socket, message);
        }
    }
}

bool ClientRegistry::sendTo(const std::string& username, const std::string& message) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (const Client& c : clients_) {
        if (c.username == username) {
            sendText(c.socket, message);
            return true;
        }
    }
    return false;
}

std::vector<std::string> ClientRegistry::usernames() {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::string> names;
    for (const Client& c : clients_) {
        names.push_back(c.username);
    }
    return names;
}
