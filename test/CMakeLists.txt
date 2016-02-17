# Add the catch unit testing framework
include_directories(${COMMON_INCLUDES} ${PROJECT_SOURCE_DIR} catch)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -Wextra -pedantic -lpthread")

set(TEST_SOURCES test_common.cpp testTSAR.cpp catch/catch.hpp ${PROJECT_SOURCE_DIR}/tsar.h)

add_executable(testTSAR ${TEST_SOURCES})
target_link_libraries(testTSAR tsar)