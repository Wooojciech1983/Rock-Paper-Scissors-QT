include(FetchContent)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)

add_executable(Tests)

target_sources(Tests
	PRIVATE
		"GameEngineTests.cpp"
)

target_include_directories(Tests PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(Tests
	PRIVATE
		Backend
		GTest::gtest
		GTest::gtest_main
)

gtest_discover_tests(Tests)
