cmake_minimum_required(VERSION 3.16)
project(second_sprint)

set(CMAKE_CXX_STANDARD 20)

#add_compile_options(-Wall -Wextra -pedantic -Werror)
set(CMAKE_CXX_FLAGS "-std=c++20")
if (EXISTS ${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    conan_basic_setup()
else ()
    message(WARNING "The file conanbuildinfo.cmake doesn't exist, you have to run conan install first")
endif ()

add_executable(second_sprint main.cpp stdafx.h document.cpp document.h paginator.h read_input_functions.cpp read_input_functions.h request_queue.cpp request_queue.h search_server.cpp search_server.h string_processing.cpp string_processing.h log_duration.h remove_duplicates.h remove_duplicates.cpp tests.cpp process_queries.cpp process_queries.h)
target_link_libraries(second_sprint ${CONAN_LIBS})