find_package(Boost 1.74 COMPONENTS iostreams REQUIRED)
include_directories(${Boost_INCLUDE_DIRS}) 

add_library(Logging logging.cpp)
target_link_libraries(Logging ${Boost_LIBRARIES})
