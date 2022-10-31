cmake_minimum_required (VERSION 3.8)
     
project(Server)
     
file(GLOB headers *.h)
file(GLOB sources *.cpp)
     
add_executable(Server ${headers} ${sources})
     
include_directories("../Common")
target_link_libraries(Server Common)
