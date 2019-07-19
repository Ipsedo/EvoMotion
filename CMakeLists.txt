cmake_minimum_required(VERSION 3.13)
project(EvoMotion)

set(CMAKE_CXX_STANDARD 11)

file(GLOB_RECURSE SRC "src/*.cpp" "src/*.c")

add_executable(EvoMotion ${SRC})