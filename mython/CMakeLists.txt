project(mython)

include(cmake/ucm.cmake)

cmake_minimum_required(VERSION 2.8.12)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)
#ucm_add_flags(
#        "-Wall"
#        "-Wpedantic"
#        "-Wextra"
#        "-fexceptions"
#        "-stdlib=libc++"
#        "-fsanitize=address"
#        "-g"
#        "-O1"
#        "-fno-omit-frame-pointer"
#        "-fno-optimize-sibling-calls"
#)

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
add_executable(mython main.cpp ${SRC} ${TESTS})
target_link_libraries(mython ${CONAN_LIBS})