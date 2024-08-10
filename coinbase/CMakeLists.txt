find_package(nlohmann_json 3.2.0 REQUIRED)
find_package(Boost 1.74 COMPONENTS log REQUIRED)
include_directories(${Boost_INCLUDE_DIRS}) 

add_library(Coinbase message.cpp subscribe.cpp)
target_link_libraries(Coinbase ${Boost_LIBRARIES})
