# TCP Chat Application in C++

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
│   ├── server.cpp    # Server — accepts connections, manages clients, broadcasts messages
│   └── client.cpp    # Client — connects to server, sends and receives messages
├── .gitignore
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
cd src
g++ server.cpp -o server -lws2_32
g++ client.cpp -o client -lws2_32
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