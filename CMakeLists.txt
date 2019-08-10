cmake_minimum_required(VERSION 3.5)
project(EvoMotion)

set(CMAKE_CXX_STANDARD 11)

find_package(Armadillo REQUIRED)
include_directories(${ARMADILLO_INCLUDE_DIRS})

find_package(Bullet REQUIRED)
include_directories(${BULLET_INCLUDE_DIRS})

file(GLOB_RECURSE SRC "src/*.cpp" "src/*.c")

add_executable(EvoMotion ${SRC})

target_link_libraries(
        EvoMotion
        ${ARMADILLO_LIBRARIES}
        ${BULLET_LIBRARIES}
)