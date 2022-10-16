cmake_minimum_required(VERSION 3.18)
project(TIGER)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -g -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function -DCMAKE_BUILD_TYPE=Debug")

find_library(YAMLCPP yaml-cpp)

set(LIB_SRC
    src/uri/uri.cc
    src/log.cc
    src/util.cc
    src/config.cc
    src/mutex.cc
    src/thread.cc
    src/coroutine.cc
    src/scheduler.cc
    src/timer.cc
    src/iomanager.cc
    src/fdmanager.cc
    src/hook.cc
)

add_library(TIGER SHARED ${LIB_SRC})

set(LIBS
    TIGER
    ${YAMLCPP}
)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

add_executable(test_uri tests/test_uri.cc)
target_link_libraries(test_uri ${LIBS})

add_executable(test_log tests/test_log.cc)
target_link_libraries(test_log ${LIBS})

add_executable(test_conf tests/test_config.cc)
target_link_libraries(test_conf ${LIBS})

add_executable(test_thread tests/test_thread.cc)
target_link_libraries(test_thread ${LIBS})

add_executable(test_mutex tests/test_mutex.cc)
target_link_libraries(test_mutex ${LIBS})

add_executable(test_coroutine tests/test_coroutine.cc)
target_link_libraries(test_coroutine ${LIBS})

add_executable(test_scheduler tests/test_scheduler.cc)
target_link_libraries(test_scheduler ${LIBS})

add_executable(test_iomanager tests/test_iomanager.cc)
target_link_libraries(test_iomanager ${LIBS})
