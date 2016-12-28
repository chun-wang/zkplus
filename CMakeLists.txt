cmake_minimum_required(VERSION 3.6)
project(zookeeperlus C CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_definitions(-D_GLIBCXX_HAS_GTHREADS)

include_directories(include/zookeeper/include)
include_directories(include)

link_directories(include/zookeeper/lib)

add_subdirectory(src)

add_executable(zookeeperlus main.cpp)

target_link_libraries(zookeeperlus zookeeper_mt pthread)

