cmake_minimum_required(VERSION 3.20)

project(order_book_simulator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

FetchContent_Declare(
  benchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG v1.8.3
)
FetchContent_MakeAvailable(benchmark)

find_package(Boost REQUIRED)
message(STATUS "Boost include dirs: ${Boost_INCLUDE_DIRS}")

add_library(order_book_engine STATIC
    src/price_level.cpp
    src/order_book_side.cpp
    src/order_pool.cpp
    src/order_lookup.cpp
    src/matching_engine.cpp
)
target_include_directories(order_book_engine
    PUBLIC include
    PUBLIC ${Boost_INCLUDE_DIRS}
)

add_executable(order_book_simulator
    src/main.cpp
)
target_link_libraries(order_book_simulator PRIVATE order_book_engine)

add_executable(order_book_bench
    benchmarks/engine_bench.cpp
)
target_link_libraries(order_book_bench
    PRIVATE
        order_book_engine
        benchmark::benchmark
        benchmark::benchmark_main
)

enable_testing()

add_executable(tests
    tests/engine_tests.cpp
)
target_link_libraries(tests
    PRIVATE
        order_book_engine
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(tests)
