cmake_minimum_required(VERSION 3.17)
project(ticker_subscriber)

set(BASEPATH "${CMAKE_SOURCE_DIR}")
include_directories("${BASEPATH}")

find_package(Boost 1.74 COMPONENTS log thread program_options REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

add_subdirectory(logging)
add_subdirectory(coinbase)

add_executable(ticker_subscriber main.cpp)
target_link_libraries(ticker_subscriber ${Boost_LIBRARIES} Logging Coinbase)
