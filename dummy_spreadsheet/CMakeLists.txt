cmake_minimum_required(VERSION 3.19)

project(dummy_spreadsheet)

include(cmake/ucm.cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)
set(ANTLR_EXECUTABLE ${CMAKE_CURRENT_SOURCE_DIR}/antlr-4.9.2-complete.jar)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/FindANTLR.cmake)
#set(
#        CMAKE_CXX_FLAGS
#        "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -Werror -Wno-unused-parameter -Wno-implicit-fallthrough"
#)
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


option(BUILD_DOC "Build documentation" ON)

# check if Doxygen is installed
find_package(Doxygen)
if (DOXYGEN_FOUND)
    # set input and output files
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/../docs/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    # request to configure the file
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
    message("Doxygen build started")

    # note the option ALL which allows to build the docs together with the application
    add_custom_target( doc_doxygen ALL
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM )
else (DOXYGEN_FOUND)
    message("Doxygen need to be installed to generate the doxygen documentation")
endif (DOXYGEN_FOUND)