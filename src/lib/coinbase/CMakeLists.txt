include(FetchContent)

FetchContent_Declare(Json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(Json)

find_package(Boost 1.74 COMPONENTS log REQUIRED)

add_subdirectory(${LIB_ROOT}/websocket websocket)
add_subdirectory(${LIB_ROOT}/logging logging)

add_library(Coinbase message.cpp subscribe.cpp ticker_logger.cpp)
target_link_libraries(Coinbase ${Boost_LIBRARIES} Websocket Logging)
