cmake_minimum_required(VERSION 3.17)
project(ticker_subscriber)

add_subdirectory(deps/boost EXCLUDE_FROM_ALL)

find_package(OpenSSL REQUIRED)

add_library(Websocket websocket.cpp)
target_link_libraries(Websocket Boost::beast Boost::asio OpenSSL::SSL)

add_subdirectory(logging)
target_link_libraries(Logging Boost::iostreams)

add_subdirectory(coinbase)
target_link_libraries(Coinbase Boost::json Boost::log Boost::beast Boost::asio Logging Websocket)

add_executable(ticker_subscriber main.cpp)
target_link_libraries(ticker_subscriber Boost::asio Boost::lockfree Boost::log Logging Coinbase)
