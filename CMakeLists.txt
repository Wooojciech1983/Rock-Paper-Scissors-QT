cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

project(HomeworkRPS)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory("Backend")
add_subdirectory("View")
add_subdirectory("RockPaperScissors")

enable_testing()
add_subdirectory("Tests")
