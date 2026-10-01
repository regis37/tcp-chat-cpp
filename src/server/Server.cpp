#include "Server.hpp"

#include "common/SocketIO.hpp"

#include <iostream>
#include <stdexcept>
#include <thread>

Server::Server(unsigned short port)
    : port_(port), logger_("chat.log"), commands_(registry_, logger_) {}

void Server::run() {
    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET) {
        throw std::runtime_error("Failed to create socket");
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port_);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        closesocket(listenSocket);
        throw std::runtime_error("Bind failed");
    }
    if (listen(listenSocket, 10) == SOCKET_ERROR) {
        closesocket(listenSocket);
        throw std::runtime_error("Listen failed");
    }
    std::cout << "Server is listening on port " << port_ << "...\n";

    while (true) {
        SOCKET clientSocket = accept(listenSocket, nullptr, nullptr);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Error: Accept failed\n";
            continue;
        }

        std::cout << "A new client has connected!\n";
        registry_.add(clientSocket);

        // detach = the thread runs on its own, the accept loop doesn't wait for it
        std::thread(&Server::handleClient, this, clientSocket).detach();
    }
}

void Server::handleClient(SOCKET clientSocket) {
    std::string username;
    if (!login(clientSocket, username)) return;

    while (true) {
        std::string message;
        if (!receiveText(clientSocket, message)) {
            std::cout << username << " has disconnected\n";
            logger_.log(username + " has left the chat");
            disconnect(clientSocket, username);
            return;
        }

        bool staying = commands_.handle(clientSocket, username, message);
        if (!staying) {
            std::cout << username << " has disconnected\n";
            disconnect(clientSocket, username);
            return;
        }
    }
}

bool Server::login(SOCKET clientSocket, std::string& username) {
    sendText(clientSocket, "Enter your username: ");

    if (!receiveText(clientSocket, username)) {
        closesocket(clientSocket);
        return false;
    }

    while (!username.empty() && (username.back() == '\n' || username.back() == '\r')) {
        username.pop_back();
    }

    // A username starting with '/' would be confused with a command
    if (username.empty() || username[0] == '/') {
        sendText(clientSocket, "Error: invalid username. Username cannot start with '/'.\n");
        registry_.remove(clientSocket);
        closesocket(clientSocket);
        return false;
    }

    registry_.setUsername(clientSocket, username);
    registry_.broadcast("*** " + username + " has joined the chat ***", clientSocket);
    sendText(clientSocket, "Welcome " + username + "! You are now connected.\n");

    std::cout << username << " has joined the chat\n";
    logger_.log(username + " has joined the chat");
    return true;
}

void Server::disconnect(SOCKET clientSocket, const std::string& username) {
    // Announce while the client is still listed, so the broadcast skips it by socket
    registry_.broadcast("*** " + username + " has left the chat ***", clientSocket);
    registry_.remove(clientSocket);
    closesocket(clientSocket);
}
