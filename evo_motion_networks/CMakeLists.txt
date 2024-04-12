cmake_minimum_required(VERSION 3.5)

project(evo_motion_networks)

set(CMAKE_CXX_STANDARD 17)

set(EVO_MOTION_NETWORKS_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/include" CACHE INTERNAL "")
include_directories("${EVO_MOTION_NETWORKS_INCLUDE_DIRS}")

# Torch
list(APPEND CMAKE_PREFIX_PATH "/opt/libtorch/share/cmake")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")
find_package(Torch REQUIRED)

# evo_motion part
# Get all source files
file(GLOB_RECURSE SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

# Create EvoMotion executable
add_library(evo_motion_networks SHARED "${SRC}")

target_include_directories(evo_motion_networks PUBLIC "${TORCH_INCLUDE_DIRS}")

# Link libraries
target_link_libraries(
        evo_motion_networks
        ${TORCH_LIBRARIES}
)

set(EVO_MOTION_NETWORKS_LIBRARIES evo_motion_networks PARENT_SCOPE)