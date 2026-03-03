#include <iostream>
#include <string>
#include <thread>
#include <atomic>        // For std::atomic — a thread-safe boolean
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

// atomic bool = a boolean that can be safely read/written by multiple threads
std::atomic<bool> running(true);

void receiveMessages(SOCKET clientSocket) {
    char buffer[1024];

    while (running) {
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);

        if (bytesReceived <= 0) {
            std::cout << "\nDisconnected from server\n";
            running = false;
            break;
        }

        buffer[bytesReceived] = '\0';

        // Clear the current line (the "> " prompt)
        // \r goes back to the beginning of the line
        // then we overwrite with spaces to erase it
        std::cout << "\r                                    \r";

        // Print the received message
        std::cout << "[Other user]: " << buffer << "\n";

        // Reprint the prompt so the user knows they can still type
        std::cout << "> " << std::flush;
    }
}

int main() {

    // STEP 1 : Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Error: WSAStartup failed\n";
        return 1;
    }

    // STEP 2 : Create socket
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Error: Failed to create socket\n";
        WSACleanup();
        return 1;
    }

    // STEP 3 : Connect to server
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(54000);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Error: Connection failed\n";
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }


    // STEP 4 : Start receiver thread
    std::thread receiver(receiveMessages, clientSocket);
    receiver.detach();

    // STEP 5 : Send messages
    std::string message;
    while (running) {
        std::cout << "> ";
        std::getline(std::cin, message);

        if (!running) break;

        if (message == "quit") {
            running = false;
            std::cout << "Disconnecting...\n";
            break;
        }

        if (!message.empty()) {
            send(clientSocket, message.c_str(), message.size(), 0);
        }
    }

    // CLEANUP
    closesocket(clientSocket);
    WSACleanup();

    return 0;
}