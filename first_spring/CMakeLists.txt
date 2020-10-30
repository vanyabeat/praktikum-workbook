cmake_minimum_required(VERSION 3.16)
project(first_spring)

set(CMAKE_CXX_STANDARD 20)

#add_compile_options(-Wall -Wextra -pedantic -Werror)
set(CMAKE_CXX_FLAGS "-fprofile-instr-generate -fcoverage-mapping -std=c++17")
add_executable(first_spring main.cpp stdafx.h)