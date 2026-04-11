# ─────────────────────────────────────────
# Server executable
# ─────────────────────────────────────────
add_executable(server server.cpp)

target_link_libraries(server ws2_32)

# ─────────────────────────────────────────
# Client executable
# ─────────────────────────────────────────
add_executable(client client.cpp)

target_link_libraries(client ws2_32)