cmake_minimum_required(VERSION 3.22.1)

project(evo_motion)

set(CMAKE_CXX_STANDARD 17)

# add evo_motion submodules
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/evo_motion_view")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/evo_motion_networks")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/evo_motion_model")

# FetchContent -> get libraries from git
include(FetchContent)

# argparse
FetchContent_Declare(
        argparse
        GIT_REPOSITORY https://github.com/p-ranav/argparse.git
)
FetchContent_MakeAvailable(argparse)

# indicators
FetchContent_Declare(
        indicators
        GIT_REPOSITORY https://github.com/p-ranav/indicators.git
)
FetchContent_MakeAvailable(indicators)

# evo_motion part
# Get all source files
file(GLOB_RECURSE SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

# Create EvoMotion executable
add_executable(evo_motion "${SRC}")

target_include_directories(evo_motion PUBLIC ${EVO_MOTION_VIEW_INCLUDE_DIRS})
target_include_directories(evo_motion PUBLIC ${EVO_MOTION_NETWORKS_INCLUDE_DIRS})
target_include_directories(evo_motion PUBLIC ${EVO_MOTION_MODEL_INCLUDE_DIRS})

# Link libraries
target_link_libraries(
        evo_motion
        argparse
        indicators
        evo_motion_view
        evo_motion_model
        evo_motion_networks
)