set(CMAKE_AUTOMOC ON)

add_library(QtView STATIC)

target_sources(QtView
    PRIVATE
        "MoveStream.h"
        "MoveStream.cpp"
        "QtView.h"
        "QtView.cpp"
        "GameWorker.h"
        "GameWorker.cpp"
        "IGameSession.h"
        "LocalGameSession.h"
        "LocalGameSession.cpp"
)

target_include_directories(QtView PUBLIC ${CMAKE_CURRENT_LIST_DIR})

# Backend provides IGameView/GameController; Qt provides the signal/slot machinery
# used by QtView and GameWorker (added in later steps).
target_link_libraries(QtView PUBLIC Backend Qt6::Core Qt6::Widgets)
