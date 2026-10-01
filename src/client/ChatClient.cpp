#include "ChatClient.hpp"

#include "common/SocketIO.hpp"

#include <ws2tcpip.h>

#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>

ChatClient::ChatClient(const char* address, unsigned short port) {
    socket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket_ == INVALID_SOCKET) {
        throw std::runtime_error("Failed to create socket");
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    inet_pton(AF_INET, address, &serverAddr.sin_addr);

    if (connect(socket_, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        // The destructor doesn't run when a constructor throws, so close here
        closesocket(socket_);
        throw std::runtime_error("Connection failed");
    }
}

ChatClient::~ChatClient() {
    closesocket(socket_);
}

void ChatClient::run() {
    std::thread receiver(&ChatClient::receiveLoop, this);

    std::string message;
    while (running_) {
        std::cout << "> ";
        std::getline(std::cin, message);

        if (!running_) break;

        if (message == "quit") {
            running_ = false;
            // Notify the server before disconnecting
            sendText(socket_, "/quit");
            std::cout << "Disconnecting...\n";
            break;
        }

        // Commands like /users are sent as-is; the server answers them
        if (!message.empty()) {
            sendText(socket_, message);
        }
    }

    // Wakes up the receiver's blocking recv so the thread can end before *this is destroyed
    shutdown(socket_, SD_BOTH);
    receiver.join();
}

void ChatClient::receiveLoop() {
    std::string message;
    while (receiveText(socket_, message)) {
        // \r goes back to the start of the line and the spaces erase the "> " prompt
        std::cout << "\r                                    \r";
        // Printed as-is: the server already formats it as "[Alice]: Hello"
        std::cout << message << "\n";
        std::cout << "> " << std::flush;
    }

    // Only report it if the server closed the connection, not the user
    if (running_) {
        std::cout << "\nDisconnected from server\n";
        running_ = false;
    }
}
