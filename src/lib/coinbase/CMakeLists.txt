find_package(nlohmann_json 3.2.0 REQUIRED)
find_package(Boost 1.74 COMPONENTS log REQUIRED)

add_subdirectory(${LIB_ROOT}/websocket websocket)
add_subdirectory(${LIB_ROOT}/logging logging)

add_library(Coinbase message.cpp subscribe.cpp ticker_logger.cpp)
target_link_libraries(Coinbase ${Boost_LIBRARIES} Websocket Logging)
