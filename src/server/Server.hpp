#pragma once

#include "ClientRegistry.hpp"
#include "CommandHandler.hpp"
#include "Logger.hpp"

#include <winsock2.h>

#include <string>

// Accepts connections and runs one thread per client
class Server {
public:
    explicit Server(unsigned short port);

    // Listens and accepts clients forever; throws if the port can't be opened
    void run();

private:
    void handleClient(SOCKET clientSocket);
    // Asks for a username and announces the client; returns false if the client is gone
    bool login(SOCKET clientSocket, std::string& username);
    void disconnect(SOCKET clientSocket, const std::string& username);

    unsigned short port_;
    // Declared before commands_, which holds references to them
    ClientRegistry registry_;
    Logger logger_;
    CommandHandler commands_;
};
