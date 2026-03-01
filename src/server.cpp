// server.cpp

#include <iostream>
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

    // STEP 2 : Create the server socket

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Error: Failed to create socket\n";
        WSACleanup();
        return 1;
    }
    std::cout << "Server socket created\n";


    // STEP 3 : Bind the socket to an address and port

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(54000);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Error: Bind failed\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }
    std::cout << "Socket bound to port 54000\n";


    // STEP 4 : Listen for incoming connections

    if (listen(serverSocket, 1) == SOCKET_ERROR) {
        std::cerr << "Error: Listen failed\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }
    std::cout << "Server is listening... Waiting for a client\n";


    // STEP 5 : Accept an incoming connection

    sockaddr_in clientAddr;
    int clientSize = sizeof(clientAddr);

    SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientSize);

    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Error: Accept failed\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }
    std::cout << "A client has connected!\n";


    // STEP 6 : Receive messages from the client

    char buffer[1024];

    while (true) {
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);

        if (bytesReceived <= 0) {
            std::cout << "Client disconnected\n";
            break;
        }

        buffer[bytesReceived] = '\0';
        std::cout << "Message received: " << buffer << "\n";
    }


    // CLEANUP

    closesocket(clientSocket);
    closesocket(serverSocket);
    WSACleanup();

    return 0;
}