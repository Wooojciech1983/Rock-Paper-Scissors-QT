add_library(View STATIC)

target_include_directories(View PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_sources(View
	PRIVATE
		"ConsoleView.h"
		"ConsoleView.cpp"
)

target_link_libraries(View PUBLIC Backend)
