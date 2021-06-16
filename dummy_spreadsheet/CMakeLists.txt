cmake_minimum_required(VERSION 3.19)

project(dummy_spreadsheet)

include(cmake/ucm.cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)
set(ANTLR_EXECUTABLE ${CMAKE_CURRENT_SOURCE_DIR}/antlr-4.9.2-complete.jar)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/FindANTLR.cmake)

add_definitions(
        -DANTLR4CPP_STATIC
        -D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/runtime)

antlr_target(FormulaParser Formula.g4 LEXER PARSER LISTENER)

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

include_directories(
        ${ANTLR4_INCLUDE_DIRS}
        ${ANTLR_FormulaParser_OUTPUT_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/runtime/src
)

add_executable(dummy_spreadsheet main.cpp ${SRC} ${TESTS} ${ANTLR_FormulaParser_CXX_OUTPUTS})

target_link_libraries(dummy_spreadsheet ${CONAN_LIBS} antlr4_static)