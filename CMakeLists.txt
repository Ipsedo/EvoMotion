cmake_minimum_required(VERSION 3.5)
project(EvoMotion)

# Bullet
find_package(Bullet REQUIRED)
include_directories(${BULLET_INCLUDE_DIRS})

# GLM
include_directories(${GLM_INCLUDE_DIRS})

# GLEW
find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})

# OpenGL
set(OpenGL_GL_PREFERENCE "LEGACY")
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS})

# GLFW-3
find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)
include_directories(${GLFW_INCLUDE_DIRS})

# SOIL
find_library(SOIL SOIL lib/libsoil.a)

# LibTorch
set(CMAKE_PREFIX_PATH "/opt/libtorch")
find_package(Torch REQUIRED)

# CLI11
# https://github.com/CLIUtils/CLI11

# Get all source files
file(GLOB_RECURSE SRC "src/*.cpp")

# Create EvoMotion executable
add_executable(EvoMotion ${SRC})

# Link libraries
target_link_libraries(
        EvoMotion
        ${BULLET_LIBRARIES}
        ${GLEW_LIBRARIES}
        ${OPENGL_LIBRARIES}
        ${GLFW_LIBRARIES}
        ${SOIL}
        ${TORCH_LIBRARIES}
)

# Add res folder to binary dir
set (source "${CMAKE_SOURCE_DIR}/res")
set (destination "${CMAKE_CURRENT_BINARY_DIR}/res")
add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink ${source} ${destination}
        DEPENDS ${destination}
        COMMENT "symbolic link resources folder from ${source} => ${destination}"
)

# Add shader folder to binary dir
set (source "${CMAKE_SOURCE_DIR}/shaders")
set (destination "${CMAKE_CURRENT_BINARY_DIR}/shaders")
add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink ${source} ${destination}
        DEPENDS ${destination}
        COMMENT "symbolic link resources folder from ${source} => ${destination}"
)