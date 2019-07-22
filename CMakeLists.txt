cmake_minimum_required(VERSION 3.5)
project(EvoMotion)

set(CMAKE_CXX_STANDARD 11)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -llapack -lblas -larmadillo")

find_package(Armadillo REQUIRED)
include_directories(${ARMADILLO_INCLUDE_DIRS})

file(GLOB_RECURSE SRC "src/*.cpp" "src/*.c")

add_executable(EvoMotion ${SRC})

target_link_libraries(EvoMotion ${ARMADILLO_LIBRARIES})