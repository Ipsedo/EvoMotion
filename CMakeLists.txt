cmake_minimum_required(VERSION 3.5)
project(EvoMotion)

set(CMAKE_CXX_STANDARD 11)

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

# Get all source files
file(GLOB_RECURSE SRC "src/*.cpp" "src/*.c")

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