cmake_minimum_required(VERSION 3.16)

project(simple_vector)

set(CMAKE_CXX_STANDARD 20)

include (ucm/ucm.cmake)


if (EXISTS ${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    conan_basic_setup()
else ()
    message(WARNING "The file conanbuildinfo.cmake doesn't exist, you have to run conan install first")
endif ()

ucm_add_flags(
    "-Wall"
    "-Wpedantic"
    "-Wextra"
    "-fexceptions"
    "-stdlib=libc++"
    # "-fsanitize=address"
    # "-g"
    # "-O1"
    # "-fno-omit-frame-pointer"
    # "-fno-optimize-sibling-calls"
    # "-fsanitize=leak"
)
add_executable(simple_vector tests.cpp simple_vector.h array_ptr.h)
target_link_libraries(simple_vector ${CONAN_LIBS})
