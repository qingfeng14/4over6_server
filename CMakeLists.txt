cmake_minimum_required(VERSION 3.6)
project(4over6)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp unp.h error_check.cpp 4over6_server.cpp 4over6_util.h 4over6_util.cpp)
add_executable(4over6 ${SOURCE_FILES})