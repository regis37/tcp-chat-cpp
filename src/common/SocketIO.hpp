#pragma once

#include "Config.hpp"

#include <winsock2.h>

#include <string>

inline void sendText(SOCKET socket, const std::string& text) {
    send(socket, text.c_str(), (int)text.size(), 0);
}

// Returns false once the connection is closed or broken.
// Builds the string from the byte count, so no '\0' terminator is needed in the buffer.
inline bool receiveText(SOCKET socket, std::string& text) {
    char buffer[kBufferSize];
    int bytes = recv(socket, buffer, sizeof(buffer), 0);
    if (bytes <= 0) return false;
    text.assign(buffer, bytes);
    return true;
}
