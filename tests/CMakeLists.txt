include(FetchContent)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# ─────────────────────────────────────────
# Behavior tests: run the real server.exe and talk to it over TCP
# ─────────────────────────────────────────
add_executable(server_behavior_tests ServerBehaviorTest.cpp)

target_link_libraries(server_behavior_tests PRIVATE GTest::gtest_main ws2_32)
target_compile_definitions(server_behavior_tests PRIVATE SERVER_EXE="$<TARGET_FILE:server>")
add_dependencies(server_behavior_tests server)

include(GoogleTest)
# Every test binds port 54000, so they can't run in parallel
gtest_discover_tests(server_behavior_tests
    DISCOVERY_TIMEOUT 30
    PROPERTIES RUN_SERIAL TRUE
)
