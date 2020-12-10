cmake_minimum_required(VERSION 3.14)
project(4)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_COMPILER clang)

set(CMAKE_CXX_FLAGS "-Wall -Wextra -std=c++17 -O2 -lstdc++")

add_executable(4 computer.h
#                 computer_example.cc
        computer_tests.cc)