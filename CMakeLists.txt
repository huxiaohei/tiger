cmake_minimum_required(VERSION 3.18)
project(TIGER)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O2 -g -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function")

set(LIB_SRC
    src/uri/uri.cc
    src/log.cc
    src/util.cc
)

add_library(TIGER SHARED ${LIB_SRC})

set(LIBS
    TIGER
)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

add_executable(test_uri tests/test_uri.cc)
target_link_libraries(test_uri ${LIBS})

add_executable(test_log tests/test_log.cc)
target_link_libraries(test_log ${LIBS})