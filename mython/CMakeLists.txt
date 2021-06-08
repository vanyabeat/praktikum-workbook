cmake_minimum_required(VERSION 3.19)
project(mython)

set(CMAKE_CXX_STANDARD 20)

add_executable(mython main.cpp lexer.h lexer.cpp lexer_test_open.cpp test_runner_p.h)