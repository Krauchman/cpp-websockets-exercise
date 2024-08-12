set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/test)

find_package(Boost COMPONENTS filesystem system unit_test_framework REQUIRED)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
FetchContent_MakeAvailable(googletest)

file(GLOB UNIT_TESTS_SRC_FILES ${PROJECT_SOURCE_DIR}/test/*.cpp)

add_executable(test_ticker_logger ${UNIT_TESTS_SRC_FILES})
target_link_libraries(test_ticker_logger ${Boost_LIBRARIES} GTest::gmock Coinbase)

enable_testing()
add_test(TickerLoggerTests test_ticker_logger)
