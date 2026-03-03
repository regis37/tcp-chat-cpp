#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <mutex>
#include <algorithm>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")


// Instead of storing just a socket, we now store
// both the socket and the username of each client
struct Client {
    SOCKET socket;
    std::string username;
};

std::vector<Client> clients;

// Mutex prevents two threads from modifying
// the same data at the same time
// Without it, two threads could write to 'clients' simultaneously
// and corrupt the data
std::mutex clientsMutex;


// BROADCAST a message to all clients except the sender
void broadcast(const std::string& message, SOCKET senderSocket) {
    std::lock_guard<std::mutex> lock(clientsMutex);
    for (Client& c : clients) {
        if (c.socket != senderSocket) {
            send(c.socket, message.c_str(), message.size(), 0);
        }
    }
}

// FUNCTION : Handle one client in its own thread
// Runs separately for each connected client

void handleClient(SOCKET clientSocket) {
    char buffer[1024];

    // ── STEP 1 : Ask for username ──
    std::string prompt = "Enter your username: ";
    send(clientSocket, prompt.c_str(), prompt.size(), 0);

    int bytes = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytes <= 0) {
        closesocket(clientSocket);
        return;
    }
    buffer[bytes] = '\0';
    std::string username(buffer);

    // Remove trailing newline or carriage return
    while (!username.empty() && (username.back() == '\n' || username.back() == '\r'))
        username.pop_back();

    // Save the username in the clients list
    {
        std::lock_guard<std::mutex> lock(clientsMutex);
        for (Client& c : clients) {
            if (c.socket == clientSocket) {
                c.username = username;
                break;
            }
        }
    }

    std::cout << username << " has joined the chat\n";

    // ── STEP 2 : Listen for messages ──
    while (true) {
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);

        if (bytesReceived <= 0) {
            std::cout << username << " has disconnected\n";

            // Remove client from list
            std::lock_guard<std::mutex> lock(clientsMutex);
            clients.erase(
                std::remove_if(clients.begin(), clients.end(),
                    [clientSocket](const Client& c) { return c.socket == clientSocket; }),
                clients.end()
            );

            closesocket(clientSocket);
            break;
        }

        buffer[bytesReceived] = '\0';
        std::string message = "[" + username + "]: " + std::string(buffer);
        std::cout << message << "\n";

        broadcast(message, clientSocket);
    }
}


int main() {

    //Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Error: WSAStartup failed\n";
        return 1;
    }
    std::cout << "Winsock initialized successfully\n";

    //Create the server socket
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Error: Failed to create socket\n";
        WSACleanup();
        return 1;
    }

    //Bind
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

    //Listen
    if (listen(serverSocket, 10) == SOCKET_ERROR) {
        std::cerr << "Error: Listen failed\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }
    std::cout << "Server is listening on port 54000...\n";

    // STEP 5 : Accept clients in a loop — this is the key change
    // Before we accepted only ONE client, now we loop forever
    // and accept as many clients as possible
    while (true) {
        sockaddr_in clientAddr;
        int clientSize = sizeof(clientAddr);

        SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientSize);

        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Error: Accept failed\n";
            continue;
        }

        std::cout << "A new client has connected!\n";

        // Add this client to the global list
        {
            std::lock_guard<std::mutex> lock(clientsMutex);
            clients.push_back(clientSocket);
        }

        // Create a new thread for this client
        // The thread will run handleClient() independently
        std::thread t(handleClient, clientSocket);
        t.detach(); // detach = the thread runs on its own, we don't wait for it
    }

    // CLEANUP

    closesocket(serverSocket);
    WSACleanup();

    return 0;
}