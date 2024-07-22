cmake_minimum_required(VERSION 3.22)
project(Cherry)

include_directories(${PROJECT_SOURCE_DIR}/src)

set(CMAKE_CXX_STANDARD 17)

add_executable(Cherry main.cpp)
