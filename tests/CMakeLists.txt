cmake_minimum_required(VERSION 3.22.1)

project(evo_motion_tests)
enable_testing()
find_package(GTest REQUIRED)

set(EVO_MOTION_TESTS_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/include" CACHE INTERNAL "")
include_directories("${EVO_MOTION_TESTS_INCLUDE_DIRS}")

file(GLOB_RECURSE SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_executable(evo_motion_tests "${SRC}")
target_link_libraries(evo_motion_tests GTest::gtest GTest::gtest_main)

add_test(
        NAME evo_motion_tests
        COMMAND evo_motion_tests
)