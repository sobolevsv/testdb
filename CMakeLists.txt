cmake_minimum_required(VERSION 3.10)
project(columndb)

set(CMAKE_CXX_STANDARD 17)

include(Interpreters/CMakeLists.txt)
include(Parser/CMakeLists.txt)
include(Server/CMakeLists.txt)
include(DataStreams/CMakeLists.txt)
include(Processors/CMakeLists.txt)
include(Libs/CMakeLists.txt)
include(Common/CMakeLists.txt)

add_executable(columndb main.cpp ${myApp_SOURCES})

target_include_directories(columndb PUBLIC ${CMAKE_CURRENT_LIST_DIR})

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(Boost 1.65 REQUIRED program_options)
target_link_libraries(columndb PRIVATE Threads::Threads ${Boost_LIBRARIES})
