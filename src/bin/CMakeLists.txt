find_package(Boost 1.74 COMPONENTS log program_options REQUIRED)

add_subdirectory(${LIB_ROOT}/logging logging)
add_subdirectory(${LIB_ROOT}/coinbase coinbase)

add_library(Config config.cpp)
target_link_libraries(Config ${Boost_PROGRAM_OPTIONS_LIBRARY})

add_executable(ticker_logger main.cpp)
target_link_libraries(ticker_logger ${Boost_LOG_LIBRARY} Logging Coinbase Config)
