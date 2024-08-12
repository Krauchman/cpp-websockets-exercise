cmake_minimum_required(VERSION 3.17)
project(ticker_logger)

set(CMAKE_CXX_STANDARD 14)

set(LIB_ROOT ${CMAKE_SOURCE_DIR}/src/lib)
find_package(Boost 1.74 REQUIRED)

include_directories(${LIB_ROOT} ${Boost_INCLUDE_DIRS})

# entrypoint
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
add_subdirectory(src/bin)

add_subdirectory(test)
