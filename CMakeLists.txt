cmake_minimum_required(VERSION 3.28)
project(HTTP)

set(CMAKE_CXX_STANDARD 17)

add_executable(HTTP main.cpp
        http_header.h
        http_header.cpp
        routing.h
        routing.cpp
)

include_directories(
        lib
)