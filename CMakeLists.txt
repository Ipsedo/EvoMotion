cmake_minimum_required(VERSION 3.5)

project(evo_motion)

set(CMAKE_CXX_STANDARD 17)

# Bullet
find_package(Bullet REQUIRED)
include_directories(${BULLET_INCLUDE_DIRS})

# Torch
list(APPEND CMAKE_PREFIX_PATH "/opt/libtorch/share/cmake")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")
find_package(Torch REQUIRED)
include_directories(${TORCH_INCLUDE_DIRS})

# GLM
find_package(glm REQUIRED)
include_directories(${GLM_INCLUDE_DIRS})

# GLEW
find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})

# OpenGL
set(OpenGL_GL_PREFERENCE "LEGACY")
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS})

# GLFW-3
find_package(glfw3 REQUIRED)
include_directories(${GLFW_INCLUDE_DIRS})

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
file(GLOB_RECURSE SRC "./src/*.cpp")

# Create EvoMotion executable
add_executable(evo_motion ${SRC})

# Link libraries
target_link_libraries(
        evo_motion
        ${BULLET_LIBRARIES}
        glm
        glfw
        OpenGL
        GLEW
        ${TORCH_LIBRARIES}
        argparse
        indicators
)

function(add_extra_folder source destination)
    add_custom_command(
            TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E create_symlink ${source} ${destination}
            DEPENDS ${destination}
            COMMENT "symbolic link resources folder from ${source} => ${destination}"
    )
endfunction()

add_extra_folder("${CMAKE_SOURCE_DIR}/shaders" "${CMAKE_CURRENT_BINARY_DIR}/shaders")
add_extra_folder("${CMAKE_SOURCE_DIR}/resources" "${CMAKE_CURRENT_BINARY_DIR}/resources")