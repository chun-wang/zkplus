cmake_minimum_required(VERSION 3.6)
project(curatorplus C CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_definitions(-D_GLIBCXX_HAS_GTHREADS)

include_directories(zookeeper/include)
link_directories(zookeeper/lib)

add_subdirectory(lock)

add_executable(curatorplus main.cpp)

target_link_libraries(curatorplus locker zookeeper_mt pthread)

