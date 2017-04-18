cmake_minimum_required(VERSION 3.6)
project(4over6)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp unp.h error_check.cpp 4over6_server.cpp 4over6_util.h 4over6_util.cpp test_client.cpp keep_alive_thread.cpp android_client_back.h android_client_back.cpp test_server_nat.cpp test_server_nat.h test_ip_queue.cpp)
add_executable(4over6 ${SOURCE_FILES})