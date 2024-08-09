cmake_minimum_required(VERSION 3.17)
project(ticker_subscriber)

add_subdirectory(deps/boost EXCLUDE_FROM_ALL)

find_package(OpenSSL REQUIRED)

add_executable(ticker_subscriber main.cpp)
target_link_libraries(ticker_subscriber Boost::beast Boost::asio Boost::json Boost::lockfree Boost::iostreams Boost::log OpenSSL::SSL)
