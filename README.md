# Rock-Paper-Scissors (Qt)

A C++20 Rock-Paper-Scissors game with a Qt6 Widgets GUI. Play locally against AI
bots (including a tournament mode), or over the network in a single room that
mixes remote human players and AI bots. The host is authoritative: it runs the
game logic and broadcasts the results, so every screen stays in sync.

## Features

- **Qt6 Widgets GUI** — setup screen, live round log, running scoreboard, move buttons.
- **Local play** — 1 human vs *N* AI opponents, with an optional "Zhejiang" win-stay/lose-shift bot.
- **Tournament mode** — single-elimination bracket for 3+ opponents.
- **Network play** — host a room or join one by IP/port. A room can contain the
  host's local human, any number of remote humans, and AI bots, all at once.
- **Host-authoritative** — the host evaluates every round (no client-side desync);
  a remote player that disconnects mid-round simply forfeits and the round resolves.
- **Tested core** — GoogleTest unit tests for the game engine (28 tests).

## Architecture

The codebase is split into independent libraries plus the GUI app. Two abstraction
"seams" in the backend let the same game logic drive every mode unchanged:

- **`IGameView`** — receives display events. `QtView` turns them into Qt signals;
  the host re-broadcasts the same events to clients over TCP.
- **`IBotStrategy`** — supplies a move. Implemented by `RandomBot`, `ZhejiangBot`,
  and `RemotePlayerStrategy` (a remote human, whose move arrives over the network).
- **`IGameSession`** — what the `GameWindow` binds to, decoupling the UI from how a
  game is run: `LocalGameSession`, `HostGameSession`, or `NetworkClientSession`.

```
Rock-Paper-Scissors-QT/
├── Backend/            # Pure game logic, no Qt
│   ├── GameEngine            # round evaluation (round-robin scoring)
│   ├── GameController        # round/score flow; injectable participants
│   ├── TournamentManager     # bracket play
│   ├── IBotStrategy / RandomBot / ZhejiangBot
│   └── IGameView, Move, RoundResult
├── QtView/             # Qt glue (static lib)
│   ├── QtView                # IGameView → Qt signals
│   ├── MoveStream            # condvar-backed istream: GUI input → blocking getline
│   ├── GameWorker            # runs the blocking game loop on a worker QThread
│   ├── IGameSession          # interface the GameWindow binds to
│   └── LocalGameSession
├── Network/            # Networking (static lib)
│   ├── Protocol              # newline-delimited JSON messages + frame reader
│   ├── NetworkServer         # host: lobby, move routing, view broadcast (TCP)
│   ├── NetworkClient         # client: replays view events, sends moves
│   ├── RemotePlayerStrategy  # a remote human as an IBotStrategy on the host
│   ├── HostGameSession
│   └── NetworkClientSession
├── RockPaperScissors/  # The GUI application
│   ├── SetupDialog, NetworkLobbyDialog, GameWindow
│   └── main.cpp
├── Tests/              # GoogleTest (fetched via FetchContent)
├── CMakeLists.txt
└── CMakePresets.json   # the "qt-mingw" preset
```

### Threading model

The backend game loop is blocking by design and runs on a dedicated `QThread`, so
the Qt event loop stays responsive. The local human's clicks reach the blocking
`std::getline` through `MoveStream` (a `condition_variable`-backed `istream`), and a
remote player's move reaches its `RemotePlayerStrategy` the same way. All view
output is delivered to the GUI thread via queued Qt signals.

### Network protocol

One compact JSON object per line over TCP. Client→host: `hello`, `move`. Host→client:
`welcome`, `lobby`, `game_started`, `your_turn`, and the broadcast view events
`log` / `scores` / `round_finished` / `game_finished`.

## Tech stack

- **Language**: C++20
- **UI**: Qt 6.11.1 — Widgets + Network (no QML)
- **Build**: CMake (presets) + Ninja
- **Compiler**: the MinGW 13.1.0 toolchain bundled with the Qt installer
  (ABI-matched to the Qt6 `mingw_64` kit)
- **Tests**: GoogleTest via CMake `FetchContent`

> This project builds with the Qt-bundled MinGW toolchain, **not** MSVC or MSYS2 GCC,
> and does **not** use Conan — GoogleTest is fetched at configure time and Qt comes
> from the installer.

## Building

### Prerequisites

Install Qt **6.11.1** with the **MinGW 13.1.0** kit via the Qt online installer.
That also provides the bundled CMake and Ninja used below. The default
[`CMakePresets.json`](CMakePresets.json) preset assumes the standard install paths:

- Compiler: `C:/Qt/Tools/mingw1310_64/bin/{gcc,g++}.exe`
- Ninja: `C:/Qt/Tools/Ninja/ninja.exe`, CMake: `C:/Qt/Tools/CMake_64/bin/cmake.exe`
- Qt prefix: `C:/Qt/6.11.1/mingw_64`

If your Qt version or location differs, adjust the paths in `CMakePresets.json`.

### Configure, build, test (PowerShell)

```powershell
$env:PATH = "C:\Qt\Tools\mingw1310_64\bin;C:\Qt\6.11.1\mingw_64\bin;C:\Qt\Tools\Ninja;" + $env:PATH
$cmake = "C:\Qt\Tools\CMake_64\bin\cmake.exe"

& $cmake --preset qt-mingw                                   # configure (build-qt/)
& $cmake --build --preset qt-mingw                           # build
& "C:\Qt\Tools\CMake_64\bin\ctest.exe" --preset qt-mingw     # run the 28 unit tests
```

In VS Code with the CMake Tools extension, just select the **`qt-mingw`** preset and
use Configure / Build / Run (see [.vscode/settings.json](.vscode/settings.json)).

## Running

```
build-qt/RockPaperScissors/RockPaperScissors.exe
```

The Qt runtime DLLs must be reachable — keep `C:\Qt\6.11.1\mingw_64\bin` on `PATH`
(the VS Code settings already do this; for a standalone copy, run `windeployqt`).

On launch the **Setup** dialog lets you pick a mode:

- **Play vs AI** — choose the number of AI opponents, optionally include the
  Zhejiang bot, and (with 3+ opponents) enable tournament mode.
- **Host Network Game** — open a lobby on a port, watch players join, then
  **Start game**. You also play locally, alongside your chosen AI bots.
- **Join Network Game** — enter the host's address and port and connect.

### Try it on one machine

1. Launch the app, choose **Host Network Game**, **Open lobby**, then **Start game**.
2. Launch a second copy, choose **Join Network Game**, connect to `127.0.0.1` and
   the same port.

The host clicks **Start round**; everyone (local human + remote players + AI bots)
plays each round simultaneously.

