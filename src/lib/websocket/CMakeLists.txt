find_package(OpenSSL REQUIRED)

add_library(Websocket websocket.cpp)
target_link_libraries(Websocket OpenSSL::SSL)
