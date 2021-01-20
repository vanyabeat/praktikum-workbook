project(linked_list)
include(cmake/ucm.cmake)
cmake_minimum_required(VERSION 2.8.12)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_BUILD_TYPE Debug)
ucm_add_flags(
        "-Wall"
        "-Wpedantic"
        "-Wextra"
        "-fexceptions"
        "-stdlib=libc++"
        "-fsanitize=address"
        "-g"
        "-O1"
        "-fno-omit-frame-pointer"
        "-fno-optimize-sibling-calls"
)

if (EXISTS ${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    conan_basic_setup()
else ()
    message(WARNING "The file conanbuildinfo.cmake doesn't exist, you have to run conan install first")
endif ()


ucm_add_dirs(tests TO TESTS)
ucm_add_dirs(src TO SRC)
add_subdirectory(linked_list)
add_subdirectory(single_linked_list)
add_subdirectory(array_ptr)
target_include_directories(array_ptr INTERFACE array_ptr/)
target_include_directories(linked_list INTERFACE linked_list/)
target_include_directories(single_linked_list INTERFACE single_linked_list/)
add_executable(main main.cpp ${SRC} ${TESTS})
target_link_libraries(main ${CONAN_LIBS} linked_list single_linked_list array_ptr)
