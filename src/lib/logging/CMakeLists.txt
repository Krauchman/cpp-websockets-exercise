find_package(Boost 1.74 COMPONENTS iostreams REQUIRED)

add_library(Logging logging.cpp)
target_link_libraries(Logging ${Boost_LIBRARIES})
