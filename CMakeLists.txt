cmake_minimum_required(VERSION 3.5)

project(evo_motion)

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

# evo_motion part

# Include headers
include_directories("./include")

# Get all source files
file(GLOB_RECURSE SRC "./src/*.cpp")

# Create EvoMotion executable
add_executable(evo_motion ${SRC})

# Link libraries
target_link_libraries(
        evo_motion
        glm
        glfw
        OpenGL
        GLEW
)

set(source "${CMAKE_SOURCE_DIR}/shaders")
set(destination "${CMAKE_CURRENT_BINARY_DIR}/shaders")
add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink ${source} ${destination}
        DEPENDS ${destination}
        COMMENT "symbolic link resources folder from ${source} => ${destination}"
)
