cmake_minimum_required(VERSION 3.17)
project(columndb)

set(CMAKE_CXX_STANDARD 17)

add_executable(columndb main.cpp Server/Server.cpp Server/Server.h)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(columndb PRIVATE Threads::Threads)
