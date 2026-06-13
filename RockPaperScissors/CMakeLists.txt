add_executable(RockPaperScissors)

# View links Backend transitively (PUBLIC), so linking View is sufficient
# to access both ConsoleView.h (View/) and GameController.h (Backend/).
target_link_libraries(RockPaperScissors PRIVATE View)

target_sources(RockPaperScissors PRIVATE "main.cpp" "AppSession.cpp")
