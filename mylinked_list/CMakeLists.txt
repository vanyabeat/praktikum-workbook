project(linked_list)
include(cmake/ucm.cmake)
cmake_minimum_required(VERSION 2.8.12)

set(CMAKE_CXX_STANDARD 20)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

set(CMAKE_BUILD_TYPE Release)
ucm_add_dirs(tests TO TESTS)
ucm_add_dirs(src TO SRC)
add_executable(main main.cpp ${SRC} ${TESTS})
target_link_libraries(main ${CONAN_LIBS})