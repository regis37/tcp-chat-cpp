#include "ChatClient.hpp"

#include "common/Config.hpp"
#include "common/WinsockSession.hpp"

#include <exception>
#include <iostream>

int main() {
    try {
        WinsockSession winsock;
        // Declared after winsock so it's destroyed first: its socket closes before WSACleanup
        ChatClient client(kServerAddress, kPort);
        client.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
