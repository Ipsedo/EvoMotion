cmake_minimum_required(VERSION 3.5)

project(evo_motion_model)

set(CMAKE_CXX_STANDARD 17)

set(RESOURCES_PATH "${CMAKE_CURRENT_BINARY_DIR}")
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/src/constants.h.in" "${CMAKE_CURRENT_BINARY_DIR}/constants.h" @ONLY)

set(EVO_MOTION_MODEL_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/include" CACHE INTERNAL "")
include_directories("${EVO_MOTION_MODEL_INCLUDE_DIRS}")

# Bullet
find_package(Bullet REQUIRED)

# Torch
list(APPEND CMAKE_PREFIX_PATH "/opt/libtorch/share/cmake")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")
find_package(Torch REQUIRED)

# GLM
find_package(glm REQUIRED)

# FetchContent -> get libraries from git
include(FetchContent)

# json
FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(json)

# evo_motion part
# Get all source files
file(GLOB_RECURSE SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

# Create EvoMotion executable
add_library(evo_motion_model SHARED "${SRC}")

target_include_directories(evo_motion_model PUBLIC "${TORCH_INCLUDE_DIRS}")
target_include_directories(evo_motion_model PUBLIC "${BULLET_INCLUDE_DIR}")
target_include_directories(evo_motion_model PUBLIC "${GLM_INCLUDE_DIRS}")
target_include_directories(evo_motion_model PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")

# Link libraries
target_link_libraries(
        evo_motion_model
        ${TORCH_LIBRARIES}
        ${BULLET_LIBRARIES}
        ${GLM_LIBRARIES}
        nlohmann_json::nlohmann_json
)

set(EVO_MOTION_MODEL_LIBRARIES evo_motion_model PARENT_SCOPE)

add_custom_target(copy-resources-files ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/resources" "${CMAKE_CURRENT_BINARY_DIR}/resources"
        DEPENDS evo_motion_model)