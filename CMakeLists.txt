cmake_minimum_required(VERSION 3.28)
project(orderbook)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_PREFIX_PATH "/opt/homebrew")

include(FetchContent)
# Download and configure GoogleTest
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/heads/main.zip
)
FetchContent_MakeAvailable(googletest)

find_package(fmt REQUIRED)

find_package(Boost REQUIRED COMPONENTS random)

add_executable(orderbook main.cpp
        include/order.h
        include/trade.h
        include/orderBook.h
        include/instrument.h
        include/TradeInfo.h
        include/commonAlias.h
        src/orderBook.cpp
        include/helper.h
        include/priceLevelInfo.h
        include/threadPool.h
        include/queue.h
        include/threadPool.h
        include/threadPool.h
        include/threadPool.h
        include/threadPool.h
        include/orderBookManager.h
)

target_link_libraries(orderbook PRIVATE fmt::fmt PRIVATE Boost::random)

## === UNIT TESTS ===
#enable_testing()
#
#add_executable(orderbook_tests
#tests/orderbook_test.cpp
#)
#
#target_link_libraries(orderbook_tests
#gtest_main
#fmt::fmt
#Boost::random
#)
#
#include(GoogleTest)
#gtest_discover_tests(orderbook_tests)