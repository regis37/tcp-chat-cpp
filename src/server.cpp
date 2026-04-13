#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <mutex>
#include <algorithm>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <sstream>
#include <windows.h>

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

// ─────────────────────────────────────────
// Returns a formatted list of all connected users
// ─────────────────────────────────────────
std::string getConnectedUsers() {
    std::lock_guard<std::mutex> lock(clientsMutex);
    std::string list = "Connected users (" + std::to_string(clients.size()) + "):\n";
    for (Client& c : clients) {
        list += "  - " + c.username + "\n";
    }
    return list;
}

// ─────────────────────────────────────────
// Returns a formatted list of available commands
// ─────────────────────────────────────────
std::string getHelpMessage() {
    std::string help = "Available commands:\n";
    help += "  /users              - List all connected users\n";
    help += "  /msg <user> <msg>   - Send a private message to a user\n";
    help += "  /help               - Show this help message\n";
    help += "  /quit               - Disconnect from the server\n";
    return help;
}


// ─────────────────────────────────────────
// Find a client socket by username
// Returns INVALID_SOCKET if not found
// ─────────────────────────────────────────
SOCKET findClientByUsername(const std::string& username) {
    for (Client& c : clients) {
        if (c.username == username) {
            return c.socket;
        }
    }
    return INVALID_SOCKET;
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

    // Reject usernames that start with '/'
    // to avoid confusion with commands
    if (username.empty() || username[0] == '/') {
        std::string error = "Error: invalid username. Username cannot start with '/'.\n";
        send(clientSocket, error.c_str(), error.size(), 0);
        closesocket(clientSocket);

        // Remove client from list
        std::lock_guard<std::mutex> lock(clientsMutex);
        clients.erase(
            std::remove_if(clients.begin(), clients.end(),
                [clientSocket](const Client& c) { return c.socket == clientSocket; }),
            clients.end()
        );
        return;
    }

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

    // Announce to everyone that this user joined
    broadcast("*** " + username + " has joined the chat ***", clientSocket);

    // Send a welcome message to the new user
    std::string welcome = "Welcome " + username + "! You are now connected.\n";
    send(clientSocket, welcome.c_str(), welcome.size(), 0);

    std::cout << username << " has joined the chat\n";

    // ── STEP 2 : Listen for messages ──
    while (true) {
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);

        if (bytesReceived <= 0) {
            std::cout << username << " has disconnected\n";

            // Announce disconnection to everyone BEFORE removing from list
            broadcast("*** " + username + " has left the chat ***", clientSocket);

            // Remove client from list
            {
                std::lock_guard<std::mutex> lock(clientsMutex);
                clients.erase(
                    std::remove_if(clients.begin(), clients.end(),
                        [clientSocket](const Client& c) { return c.socket == clientSocket; }),
                    clients.end()
                );
            }

            closesocket(clientSocket);
            break;
        }

        buffer[bytesReceived] = '\0';
        std::string message(buffer);

        if (message == "/users") {
            std::string userList = getConnectedUsers();
            send(clientSocket, userList.c_str(), userList.size(), 0);

        } else if (message == "/help") {
            std::string help = getHelpMessage();
            send(clientSocket, help.c_str(), help.size(), 0);

        } else if (message == "/quit") {
            std::string bye = "Goodbye " + username + "!\n";
            send(clientSocket, bye.c_str(), bye.size(), 0);
            // Small delay to ensure the message is sent before closing
            Sleep(100);
            closesocket(clientSocket);
            break;
        
        } else if (message.substr(0, 4) == "/msg") {
            // Guard against "/msg" with nothing after it
            if (message.size() < 6) {
                std::string error = "Usage: /msg <username> <message>\n";
                send(clientSocket, error.c_str(), error.size(), 0);
            } else {
            std::istringstream iss(message.substr(5));
            std::string target;
            std::string privateMessage;
        
            if (!(iss >> target)) {
                // No username provided
                std::string error = "Usage: /msg <username> <message>\n";
                send(clientSocket, error.c_str(), error.size(), 0);
            } else if (!(iss >> privateMessage)) {
                // No message provided
                std::string error = "Usage: /msg <username> <message>\n";
                send(clientSocket, error.c_str(), error.size(), 0);
            } else {
                // Get the rest of the message after the username
                std::string rest;
                std::getline(iss, rest);
                privateMessage += rest;
        
                // Find the target client
                SOCKET targetSocket = findClientByUsername(target);
        
                if (targetSocket == INVALID_SOCKET) {
                    // User not found
                    std::string error = "Error: user \"" + target + "\" not found\n";
                    send(clientSocket, error.c_str(), error.size(), 0);
                } else {
                    // Send to target
                    std::string toTarget = "[PM from " + username + "]: " + privateMessage;
                    send(targetSocket, toTarget.c_str(), toTarget.size(), 0);
        
                    // Confirm to sender
                    std::string toSender = "[PM to " + target + "]: " + privateMessage;
                    send(clientSocket, toSender.c_str(), toSender.size(), 0);
        
                    std::cout << "[PM] " << username << " -> " << target << ": " << privateMessage << "\n";
                }
            }
        }// close the else of size check
        
        } else {
            std::string formatted = "[" + username + "]: " + message;
            std::cout << formatted << "\n";
            broadcast(formatted, clientSocket);
        }
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
            clients.push_back({clientSocket, ""});
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