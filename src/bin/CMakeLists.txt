find_package(Boost 1.74 COMPONENTS program_options REQUIRED)

add_subdirectory(${LIB_ROOT}/coinbase coinbase)

add_executable(ticker_logger main.cpp)
target_link_libraries(ticker_logger ${Boost_LIBRARIES} Coinbase)
