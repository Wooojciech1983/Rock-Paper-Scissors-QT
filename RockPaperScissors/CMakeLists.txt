set(CMAKE_AUTOMOC ON)

# WIN32 keeps the GUI app from spawning a console window on Windows.
add_executable(RockPaperScissors WIN32)

target_sources(RockPaperScissors
    PRIVATE
        "main.cpp"
        "SetupDialog.h"
        "SetupDialog.cpp"
        "GameWindow.h"
        "GameWindow.cpp"
        "NetworkLobbyDialog.h"
        "NetworkLobbyDialog.cpp"
)

# Network brings QtView (and Backend, Qt6 Widgets/Network) in transitively; the
# Qt modules are also listed explicitly for clarity.
target_link_libraries(RockPaperScissors PRIVATE Network QtView Qt6::Widgets Qt6::Network)
