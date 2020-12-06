cmake_minimum_required(VERSION 3.16)
project(second_sprint)

set(CMAKE_CXX_STANDARD 20)

#add_compile_options(-Wall -Wextra -pedantic -Werror)
set(CMAKE_CXX_FLAGS "-fprofile-instr-generate -fcoverage-mapping -std=c++17")
add_executable(second_sprint main.cpp stdafx.h document.cpp document.h paginator.h read_input_functions.cpp read_input_functions.h request_queue.cpp request_queue.h search_server.cpp search_server.h string_processing.cpp string_processing.h log_duration.h)