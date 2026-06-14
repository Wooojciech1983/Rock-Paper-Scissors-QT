set(CMAKE_AUTOMOC ON)

add_library(Network STATIC)

target_sources(Network
    PRIVATE
        "Protocol.h"
        "Protocol.cpp"
        "RemotePlayerStrategy.h"
        "RemotePlayerStrategy.cpp"
        "NetworkServer.h"
        "NetworkServer.cpp"
        "HostGameSession.h"
        "HostGameSession.cpp"
        "NetworkClient.h"
        "NetworkClient.cpp"
        "NetworkClientSession.h"
        "NetworkClientSession.cpp"
)

target_include_directories(Network PUBLIC ${CMAKE_CURRENT_LIST_DIR})

# Backend provides IBotStrategy/Move; Qt6::Network drives the TCP transport and
# Qt6::Core the JSON protocol. QtView provides IGameSession/QtView/GameWorker/
# MoveStream for the host session (and pulls in Qt6::Widgets transitively).
target_link_libraries(Network PUBLIC Backend QtView Qt6::Core Qt6::Network)
