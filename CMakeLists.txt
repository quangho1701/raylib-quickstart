cmake_minimum_required(VERSION 3.14)
project(my-awesome-game)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 5.0
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(raylib)

add_executable(${PROJECT_NAME} src/main.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE raylib)

if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE winmm)
endif()
