cmake_minimum_required(VERSION 3.19)

project(dummy_spreadsheet)

include(cmake/ucm.cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)
set(ANTLR_EXECUTABLE ${CMAKE_CURRENT_SOURCE_DIR}/antlr-4.9.2-complete.jar)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/FindANTLR.cmake)

if (EXISTS ${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    conan_basic_setup()
else ()
    message(WARNING "The file conanbuildinfo.cmake doesn't exist, you have to run conan install first")
endif ()

ucm_add_dirs(tests TO TESTS)

ucm_add_dirs(src TO SRC)

include_directories(src)

include_directories(tests)

add_executable(dummy_spreadsheet main.cpp ${SRC} ${TESTS})

target_link_libraries(dummy_spreadsheet ${CONAN_LIBS})