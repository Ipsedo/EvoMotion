cmake_minimum_required(VERSION 3.5)

project(evo_motion_view)
set(CMAKE_CXX_STANDARD 17)

set(SHADERS_SOURCE_PATH "${CMAKE_CURRENT_BINARY_DIR}")
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/src/constants.h.in" "${CMAKE_CURRENT_BINARY_DIR}/constants.h" @ONLY)

set(EVO_MOTION_VIEW_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/include" CACHE INTERNAL "")
include_directories(${EVO_MOTION_VIEW_INCLUDE_DIRS})

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
set(IMGUI_BUILD_GLFW_BINDING ON)
set(IMGUI_BUILD_OPENGL3_BINDING ON)
find_package(imgui REQUIRED)

# evo_motion part
# Get all source files
file(GLOB_RECURSE SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

# Create EvoMotion executable
add_library(evo_motion_view SHARED "${SRC}")

target_include_directories(evo_motion_view PUBLIC ${GLM_INCLUDE_DIRS})
target_include_directories(evo_motion_view PUBLIC ${GLEW_INCLUDE_DIRS})
target_include_directories(evo_motion_view PUBLIC ${OPENGL_INCLUDE_DIRS})
target_include_directories(evo_motion_view PUBLIC ${GLFW_INCLUDE_DIRS})
target_include_directories(evo_motion_view PUBLIC ${IMGUI_INCLUDE_DIRS})
target_include_directories(evo_motion_view PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")

# Link libraries
target_link_libraries(
        evo_motion_view
        ${GLM_LIBRARIES}
        OpenGL
        GLEW
        glfw
        imgui
)

set(EVO_MOTION_VIEW_LIBRARIES evo_motion_view PARENT_SCOPE)

add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders" "${CMAKE_CURRENT_BINARY_DIR}/shaders"
        DEPENDS ${destination}
        COMMENT "symbolic link resources folder from ${source} => ${destination}"
)
