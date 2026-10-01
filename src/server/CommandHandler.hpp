#pragma once

#include "ClientRegistry.hpp"
#include "Logger.hpp"

#include <winsock2.h>

#include <string>

// Reacts to one message from a logged-in client: a command (/users, /help, /msg, /quit)
// or a chat message to broadcast
class CommandHandler {
public:
    CommandHandler(ClientRegistry& registry, Logger& logger);

    // Returns false when the client asked to leave (/quit)
    bool handle(SOCKET socket, const std::string& username, const std::string& message);

private:
    void listUsers(SOCKET socket);
    void showHelp(SOCKET socket);
    void sayGoodbye(SOCKET socket, const std::string& username);
    void sendPrivateMessage(SOCKET socket, const std::string& username, const std::string& message);
    void broadcastChat(SOCKET socket, const std::string& username, const std::string& message);

    ClientRegistry& registry_;
    Logger& logger_;
};
