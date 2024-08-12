cmake_minimum_required(VERSION 3.17)
project(ticker_logger)

set(BASEPATH "${CMAKE_SOURCE_DIR}")
include_directories("${BASEPATH}")

find_package(Boost 1.74 COMPONENTS log program_options REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

add_subdirectory(logging)
add_subdirectory(coinbase)

add_library(Config config.cpp)
target_link_libraries(Config ${Boost_PROGRAM_OPTIONS_LIBRARY})

add_executable(ticker_logger main.cpp)
target_link_libraries(ticker_logger ${Boost_LOG_LIBRARY} Logging Coinbase Config)
