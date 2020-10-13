cmake_minimum_required(VERSION 3.16)
project(first_spring)

set(CMAKE_CXX_FLAGS "-fprofile-instr-generate -fcoverage-mapping -std=c++17")

add_executable(first_spring main.cpp)