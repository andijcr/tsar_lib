cmake_minimum_required(VERSION 2.8)
project(TSAR-ThreadSAfeRouter)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11 -Wall -Wextra -pedantic")

add_library(list_hw2_generic STATIC lib/hwc2list/list.c)

add_library(inmem_logger STATIC lib/inmemory_logger.c)

find_package(Threads REQUIRED)

add_library(tsar STATIC tsar.h tsar.c)
target_link_libraries(tsar inmem_logger)
target_link_libraries(tsar list_hw2_generic)
target_link_libraries(tsar Threads::Threads)

# test subdirectory
add_subdirectory(test)
