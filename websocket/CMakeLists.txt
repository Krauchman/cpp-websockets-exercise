find_package(OpenSSL REQUIRED)
find_package(Boost 1.74 REQUIRED)
include_directories(${Boost_INCLUDE_DIRS}) 

add_library(Websocket websocket.cpp)
target_link_libraries(Websocket OpenSSL::SSL)
