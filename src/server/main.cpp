#include "Server.hpp"

#include "common/Config.hpp"
#include "common/WinsockSession.hpp"

#include <exception>
#include <iostream>

int main() {
    try {
        WinsockSession winsock;
        std::cout << "Winsock initialized successfully\n";

        Server server(kPort);
        server.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
