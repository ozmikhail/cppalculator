cmake_minimum_required(VERSION 3.20)
project(cppalculator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(core)

add_executable(calc main.cpp)
target_link_libraries(calc PRIVATE calc_core)
