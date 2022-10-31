cmake_minimum_required (VERSION 3.8)
     
project(Client)
     
file(GLOB headers *.h)
file(GLOB sources *.cpp)
     
add_executable(Client ${headers} ${sources})
     
include_directories("../Common")
target_link_libraries(Client Common)
