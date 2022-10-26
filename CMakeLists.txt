cmake_minimum_required(VERSION 3.18)
project(TIGER)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -g -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function -DCMAKE_BUILD_TYPE=Debug")

find_library(YAMLCPP yaml-cpp)

set(LIB_SRC
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
    src/address.cc
    src/socket.cc
    src/bytearray.cc
    src/streams/stream.cc
    src/streams/socket_stream.cc
    src/servers/tcp_server.cc
    src/uri.cc
    src/servers/http/http_response_parser.cc
    src/servers/http/http_request_parser.cc
    src/servers/http/http.cc
    src/servers/http/http_session.cc
    src/servers/http/http_parser.cc
    src/servers/http/http_connection.cc
    src/servers/http/http_servlet.cc
    src/servers/http/http_server.cc
)

add_library(TIGER SHARED ${LIB_SRC})

set(LIBS
    TIGER
    ${YAMLCPP}
)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

add_executable(test_log tests/test_log.cc)
target_link_libraries(test_log ${LIBS})

add_executable(test_thread tests/test_thread.cc)
target_link_libraries(test_thread ${LIBS})

add_executable(test_mutex tests/test_mutex.cc)
target_link_libraries(test_mutex ${LIBS})

add_executable(test_coroutine tests/test_coroutine.cc)
target_link_libraries(test_coroutine ${LIBS})

add_executable(test_iomanager tests/test_iomanager.cc)
target_link_libraries(test_iomanager ${LIBS})

add_executable(test_address tests/test_address.cc)
target_link_libraries(test_address ${LIBS})

add_executable(test_socket tests/test_socket.cc)
target_link_libraries(test_socket ${LIBS})

add_executable(test_bytearray tests/test_bytearray.cc)
target_link_libraries(test_bytearray ${LIBS})

add_executable(test_tcpserver tests/test_tcpserver.cc)
target_link_libraries(test_tcpserver ${LIBS})

add_executable(test_http_parser tests/test_http_parser.cc)
target_link_libraries(test_http_parser ${LIBS})

add_executable(test_http_connection tests/test_http_connection.cc)
target_link_libraries(test_http_connection ${LIBS})

add_executable(test_http_server tests/test_http_server.cc)
target_link_libraries(test_http_server ${LIBS})
