add_library(Backend STATIC)

target_include_directories(Backend PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_sources(Backend
	PRIVATE
		"Move.h"
		"GameEngine.h"
		"GameEngine.cpp"
		"IGameView.h"
		"IBotStrategy.h"
		"RandomBot.h"
		"RandomBot.cpp"
		"ZhejiangBot.h"
		"ZhejiangBot.cpp"
		"GameController.h"
		"GameController.cpp"
		"TournamentParticipant.h"
		"TournamentManager.h"
		"TournamentManager.cpp"
)
