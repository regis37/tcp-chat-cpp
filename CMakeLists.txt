cmake_minimum_required(VERSION 3.15)

# Project name and language
project(TCPChat VERSION 1.0 LANGUAGES CXX)

# Use C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add the src subdirectory
add_subdirectory(src)