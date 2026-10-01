#include "CommandHandler.hpp"

#include "common/SocketIO.hpp"

#include <chrono>
#include <iostream>
#include <sstream>
#include <thread>

namespace {

const std::string kMsgUsage = "Usage: /msg <username> <message>\n";

} // namespace

CommandHandler::CommandHandler(ClientRegistry& registry, Logger& logger)
    : registry_(registry), logger_(logger) {}

bool CommandHandler::handle(SOCKET socket, const std::string& username, const std::string& message) {
    if (message == "/users") {
        listUsers(socket);
    } else if (message == "/help") {
        showHelp(socket);
    } else if (message == "/quit") {
        sayGoodbye(socket, username);
        return false;
    } else if (message.substr(0, 4) == "/msg") {
        sendPrivateMessage(socket, username, message);
    } else {
        broadcastChat(socket, username, message);
    }
    return true;
}

void CommandHandler::listUsers(SOCKET socket) {
    std::vector<std::string> names = registry_.usernames();
    std::string list = "Connected users (" + std::to_string(names.size()) + "):\n";
    for (const std::string& name : names) {
        list += "  - " + name + "\n";
    }
    sendText(socket, list);
}

void CommandHandler::showHelp(SOCKET socket) {
    std::string help = "Available commands:\n";
    help += "  /users              - List all connected users\n";
    help += "  /msg <user> <msg>   - Send a private message to a user\n";
    help += "  /help               - Show this help message\n";
    help += "  /quit               - Disconnect from the server\n";
    sendText(socket, help);
}

void CommandHandler::sayGoodbye(SOCKET socket, const std::string& username) {
    sendText(socket, "Goodbye " + username + "!\n");
    // Give the goodbye time to reach the client before the server closes the socket
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

void CommandHandler::sendPrivateMessage(SOCKET socket, const std::string& username, const std::string& message) {
    // "/msg" alone, or "/msg " with nothing after it
    if (message.size() < 6) {
        sendText(socket, kMsgUsage);
        return;
    }

    std::istringstream iss(message.substr(5));
    std::string target;
    std::string text;
    if (!(iss >> target) || !(iss >> text)) {
        sendText(socket, kMsgUsage);
        return;
    }
    // `iss >> text` only read the first word; append the rest of the line
    std::string rest;
    std::getline(iss, rest);
    text += rest;

    if (!registry_.sendTo(target, "[PM from " + username + "]: " + text)) {
        sendText(socket, "Error: user \"" + target + "\" not found\n");
        return;
    }

    sendText(socket, "[PM to " + target + "]: " + text);
    std::cout << "[PM] " << username << " -> " << target << ": " << text << "\n";
    logger_.log("[PM] " + username + " -> " + target + ": " + text);
}

void CommandHandler::broadcastChat(SOCKET socket, const std::string& username, const std::string& message) {
    std::string formatted = "[" + username + "]: " + message;
    std::cout << formatted << "\n";
    logger_.log(formatted);
    registry_.broadcast(formatted, socket);
}
