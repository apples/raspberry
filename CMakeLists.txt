cmake_minimum_required (VERSION 3.0)
project(Raspberry)

file(GLOB_RECURSE raspberry_SOURCES include/raspberry/*)
add_library(raspberry INTERFACE)
set_property(TARGET raspberry PROPERTY INTERFACE_SOURCES ${raspberry_SOURCES})
target_include_directories(raspberry INTERFACE include)

add_executable(test_raspberry EXCLUDE_FROM_ALL src/test.cpp)
set_property(TARGET test_raspberry PROPERTY CXX_STANDARD 14)
target_link_libraries(test_raspberry raspberry)
