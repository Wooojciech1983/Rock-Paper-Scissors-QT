# Rock-Paper-Scissors Multiplayer Network Game

A modern C++20 multiplayer game demonstrating real-time network communication, game logic, and professional Qt Quick UI.

## Features

- **UDP Networking**: Client-server architecture with custom binary protocol
- **Real-time Gameplay**: 1v1 matches with low-latency packet handling
- **Modern C++**: C++20 features (std::optional, std::variant, move semantics)
- **Qt Quick/QML**: Declarative UI built with Qt6
- **Professional Stack**: CMake + Conan + MSVC

## Architecture

```
rps-network/
├── src/
│   ├── game/           # Game logic (Player, Move, Result)
│   ├── network/        # UDP Server/Client, Protocol
│   ├── ui/             # Qt Controller (QML bridge)
│   └── main.cpp
├── qml/
│   └── main.qml        # Qt Quick interface
├── CMakeLists.txt
└── conanfile.txt
```

## Tech Stack

- **Language**: C++20
- **UI Framework**: Qt 6.11.1 with QML
- **Networking**: UDP sockets (custom protocol)
- **Build**: CMake 3.25+
- **Package Manager**: Conan 2.x
- **Compiler**: MSVC 2022+ / GCC 11+

## Building

### Prerequisites
- Qt 6.11+ (MSVC build)
- CMake 3.25+
- Conan 2.x

### Setup

```bash
cd C:\Users\wojci\source\repos\rps-network-game

# Install dependencies
conan install . --build=missing

# Configure
cmake --preset=default

# Build
cmake --build build --config Release
```

## Usage

### Start Server
```bash
./build/Release/rps-server.exe
```

### Start Client
```bash
./build/Release/rps-client.exe
```

Connect, enter your move, and play!

## Game Flow

1. **Server listening** on UDP port 9999
2. **Client connects** and sends player name
3. **Players submit moves** (Rock/Paper/Scissors)
4. **Server evaluates** and returns result
5. **UI displays** winner and score

## Demonstrating

- Real-time network protocol design
- Modern C++ patterns (RAII, move semantics, smart pointers)
- Game state management
- Cross-platform Qt development
- Professional build configuration

## License

MIT
