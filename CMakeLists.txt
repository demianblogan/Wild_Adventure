cmake_minimum_required(VERSION 3.21)

project(Platformer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SFML 3 COMPONENTS Graphics Window System Audio CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

add_executable(Platformer
    src/main.cpp
    src/Application.h
    src/Application.cpp
	src/Context.h
	src/core/State.h
	src/core/StateMachine.h
	src/core/StateMachine.cpp
 "src/states/TestState.h" "src/states/TestState.cpp")

target_link_libraries(Platformer PRIVATE
    SFML::Graphics
    SFML::Window
    SFML::System
    SFML::Audio
    nlohmann_json::nlohmann_json
)

target_include_directories(Platformer PRIVATE src)
