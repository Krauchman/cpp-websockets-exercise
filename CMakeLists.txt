cmake_minimum_required(VERSION 3.17)
project(ticker_subscriber)

find_package(Boost 1.74 COMPONENTS log thread REQUIRED)
include_directories(${Boost_INCLUDE_DIRS}) 

add_subdirectory(coinbase)

add_executable(ticker_subscriber main.cpp)
target_link_libraries(ticker_subscriber ${Boost_LIBRARIES} Logging Coinbase)
