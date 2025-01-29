cmake_minimum_required(VERSION 3.22.1)

project(evo_motion)

set(CMAKE_CXX_STANDARD 20)

# add evo_motion submodules
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/evo_motion_view")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/evo_motion_networks")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/evo_motion_model")

# add evo_motion tests submodule
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/evo_motion_networks/tests")
# add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/tests")

# GLM
find_package(glm REQUIRED)

# GLEW
find_package(GLEW REQUIRED)

# OpenGL
set(OpenGL_GL_PREFERENCE "LEGACY")
find_package(OpenGL REQUIRED)

# GLFW-3
find_package(glfw3 REQUIRED)

# ImGUI
find_package(imgui REQUIRED)

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

# ImGui File browser
FetchContent_Declare(
        imgui-file-browser
        GIT_REPOSITORY https://github.com/AirGuanZ/imgui-filebrowser.git
)
FetchContent_MakeAvailable(imgui-file-browser)

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
        ${GLM_LIBRARIES}
        OpenGL
        GLEW
        glfw
        imgui
        evo_motion_view
        evo_motion_model
        evo_motion_networks
)

target_include_directories(evo_motion PRIVATE "${FETCHCONTENT_BASE_DIR}/imgui-file-browser-src")