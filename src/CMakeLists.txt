# ─────────────────────────────────────────
# Server executable
# ─────────────────────────────────────────
add_executable(server
    server/main.cpp
    server/Server.cpp
    server/ClientRegistry.cpp
    server/CommandHandler.cpp
    server/Logger.cpp
)

# Lets files write #include "common/Config.hpp" from any folder
target_include_directories(server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(server ws2_32)

# ─────────────────────────────────────────
# Client executable
# ─────────────────────────────────────────
add_executable(client
    client/main.cpp
    client/ChatClient.cpp
)

target_include_directories(client PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(client ws2_32)
