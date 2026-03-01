// client.cpp

#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

int main() {


    // STEP 1 : Initialize Winsock

    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Error: WSAStartup failed\n";
        return 1;
    }
    std::cout << "Winsock initialized successfully\n";


    // STEP 2 : Create the client socket

    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Error: Failed to create socket\n";
        WSACleanup();
        return 1;
    }
    std::cout << "Client socket created\n";


    // STEP 3 : Connect to the server

    // We tell the client where the server is :
    // IP  : 127.0.0.1 = localhost (my own PC for testing)
    // Port: 54000
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(54000);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Error: Connection to server failed\n";
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }
    std::cout << "Connected to server!\n";
    std::cout << "Type your messages below (type 'quit' to exit):\n\n";


    // STEP 4 : Send messages to the server

    std::string message;

    while (true) {
        std::cout << "> ";
        std::getline(std::cin, message);  // Read what the user types

        // If user types 'quit', we stop
        if (message == "quit") {
            std::cout << "Disconnecting...\n";
            break;
        }

        // Send the message to the server
        int bytesSent = send(clientSocket, message.c_str(), message.size(), 0);

        if (bytesSent == SOCKET_ERROR) {
            std::cerr << "Error: Failed to send message\n";
            break;
        }
    }


    // CLEANUP

    closesocket(clientSocket);
    WSACleanup();

    return 0;
}