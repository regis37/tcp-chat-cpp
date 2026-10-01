cmake_minimum_required(VERSION 3.15)

# Project name and language
project(TCPChat VERSION 1.0 LANGUAGES CXX)

# Use C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Bundle the MinGW runtime into each .exe so it doesn't depend on
# whichever libstdc++-6.dll comes first on PATH (e.g. Git for Windows ships its own)
if(MINGW)
    add_link_options(-static)
endif()

# Add the src subdirectory
add_subdirectory(src)

option(TCPCHAT_BUILD_TESTS "Build the test suite" ON)
if(TCPCHAT_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()