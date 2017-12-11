cmake_minimum_required(VERSION 3.8)
project(hw3)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

set(SOURCE_FILES main.cpp)
add_executable(hw3 ${SOURCE_FILES})