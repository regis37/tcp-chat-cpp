# TCP Chat Application in C++

![C++](https://img.shields.io/badge/C++-17-blue.svg)
![CMake](https://img.shields.io/badge/CMake-3.15+-green.svg)
![Platform](https://img.shields.io/badge/platform-Windows-lightgrey.svg)
![License](https://img.shields.io/badge/license-MIT-green.svg)

A multi-client terminal chat application built in C++ using Winsock and POSIX-style TCP sockets. Clients connect to a central server, choose a username, and exchange messages in real time.

---

## Features

- Multi-client support using threads (`std::thread`)
- Real-time message broadcasting to all connected clients
- Username system — each client identifies itself on connection
- Join and leave announcements
- Thread-safe client list using `std::mutex`
- Graceful disconnection with `/quit` signal
- `/users` command — list all connected users
- `/msg <username> <message>` — private messaging between users
- CMake build system for easy compilation

## Concepts Covered

| Concept | Description |
|---|---|
| TCP Sockets | Reliable, connection-based communication between processes |
| Winsock | Windows API for network programming |
| Multithreading | One thread per client to handle connections in parallel |
| Mutex | Prevents race conditions when multiple threads access shared data |
| Broadcast | Server forwards each message to all connected clients |

---

## Project Structure
```
tcp-chat-cpp/
├── src/
│   ├── common/                  # Shared by client and server
│   │   ├── Config.hpp           # Port, server address, buffer size
│   │   ├── SocketIO.hpp         # sendText / receiveText helpers
│   │   └── WinsockSession.hpp   # Starts and stops Winsock (RAII)
│   ├── server/
│   │   ├── main.cpp
│   │   ├── Server.hpp/.cpp          # Listens, accepts clients, one thread per client
│   │   ├── ClientRegistry.hpp/.cpp  # Connected clients, guarded by a mutex
│   │   ├── CommandHandler.hpp/.cpp  # /users, /help, /msg, /quit and chat messages
│   │   └── Logger.hpp/.cpp          # Thread-safe chat.log writer
│   └── client/
│       ├── main.cpp
│       └── ChatClient.hpp/.cpp      # Connects, receives in a thread, sends user input
├── tests/
│   ├── ServerBehaviorTest.cpp   # Runs the real server and drives it over TCP
│   └── support/                 # Test client and server process helpers
├── CMakeLists.txt
└── README.md
```

---

## Getting Started

### Requirements

- Windows OS
- CMake 3.15 or higher
- g++ compiler (MinGW or similar)
- Winsock2 (included with Windows SDK)

### Build with CMake (recommended)

```bash
mkdir build
cd build
cmake .. -G "MinGW Makefiles"
cmake --build .
```

Executables will be generated in `build/src/`.

### Build manually (alternative)

```bash
g++ -std=c++17 -Isrc src/server/*.cpp -o server -lws2_32 -static
g++ -std=c++17 -Isrc src/client/*.cpp -o client -lws2_32 -static
```

### Run the tests

The test suite uses [GoogleTest](https://github.com/google/googletest), downloaded automatically by CMake. Each test starts the real server and talks to it over TCP, so make sure no other server is running on port 54000.

```bash
cmake --build build
ctest --test-dir build --output-on-failure
```

### Run

Open two separate terminals and navigate to `build/src/`.

**Terminal 1 — Start the server:**
```bash
./server
```

**Terminal 2 and 3 — Start clients:**
```bash
./client
```

Each client will be prompted to enter a username. Once connected, messages are broadcast to all other clients in real time.

## Example
```
Server terminal:
  Server is listening on port 54000...
  A new client has connected!
  Regis has joined the chat
  JC has joined the chat
  [Regis]: Hello JC!
  [JC]: Hey Regis!

Client 1 (Regis):
  Enter your username: Regis
  Welcome Regis! You are now connected.
  *** JC has joined the chat ***
  > Hello JC!
  [JC]: Hey Regis!

Client 2 (JC):
  Enter your username: JC
  Welcome JC! You are now connected.
  [Regis]: Hello JC!
  > Hey Regis!
```

---

## Future Improvements

- Cross-platform support (Linux/macOS using POSIX sockets)
- GUI interface using Qt
- Message history — save chat logs to a file
- Authentication — password protected rooms

---

## Author

**Regis Tsafack**  
GitHub: [github.com/regis37](https://github.com/regis37)