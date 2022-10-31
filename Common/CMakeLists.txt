cmake_minimum_required(VERSION 3.8)
project(Common VERSION 0.1.0)

include(CTest)
enable_testing()

include_directories(${PROJECT_SOURCE_DIR} )
include_directories(${PROJECT_SOURCE_DIR}/Common )

file(GLOB headers ${PROJECT_SOURCE_DIR}/*.h)
file(GLOB sources ${PROJECT_SOURCE_DIR}/*.cpp)
add_library(Common STATIC ${headers} ${sources})
