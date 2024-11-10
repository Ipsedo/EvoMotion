cmake_minimum_required(VERSION 3.22.1)

project(evo_motion_networks_tests)
enable_testing()
find_package(GTest REQUIRED)

set(EVO_MOTION_NETWORKS_TESTS_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/include" CACHE INTERNAL "")
include_directories("${EVO_MOTION_NETWORKS_TESTS_INCLUDE_DIRS}")

# add evo_motion_networks headers
file(GLOB_RECURSE SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_executable(evo_motion_networks_tests "${SRC}")

target_link_libraries(evo_motion_networks_tests GTest::gtest GTest::gtest_main evo_motion_networks)
target_include_directories(evo_motion_networks_tests PRIVATE "${EVO_MOTION_NETWORKS_INCLUDE_DIRS}")

add_test(
        NAME evo_motion_networks_tests
        COMMAND evo_motion_networks_tests
)