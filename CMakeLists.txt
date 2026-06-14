cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

project(HomeworkRPS LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Qt6 powers the GUI (Widgets) and networking (Network). AUTOMOC is enabled
# per-target in the QtView/, Network/ and RockPaperScissors/ subdirectories so
# Backend and the GoogleTest sources are not needlessly scanned by moc.
# The Qt install prefix is supplied via CMAKE_PREFIX_PATH (see CMakePresets.json).
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network)

add_subdirectory("Backend")
add_subdirectory("QtView")
add_subdirectory("Network")
add_subdirectory("RockPaperScissors")

enable_testing()
add_subdirectory("Tests")
