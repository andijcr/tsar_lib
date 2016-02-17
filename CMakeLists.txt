cmake_minimum_required(VERSION 2.8)
project(TSAR-ThreadSAfeRouter)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 -lpthread -Wall -Wextra -pedantic")

add_library(inmem_logger STATIC lib/inmemory_logger.h lib/inmemory_logger.c)

add_library(tsar STATIC tsar.h tsar.c)
target_link_libraries(tsar inmem_logger)

# test subdirectory
add_subdirectory(test)
