cmake_minimum_required (VERSION 3.0)
project(Raspberry)

file(GLOB_RECURSE raspberry_SOURCES include/raspberry/*)
add_library(raspberry INTERFACE)
set_property(TARGET raspberry PROPERTY INTERFACE_SOURCES ${raspberry_SOURCES})
target_include_directories(raspberry INTERFACE include)

file(GLOB_RECURSE raspberry_test_SOURCES test/*)
add_executable(test_raspberry EXCLUDE_FROM_ALL ${raspberry_test_SOURCES})
set_property(TARGET test_raspberry PROPERTY CXX_STANDARD 14)
target_link_libraries(test_raspberry raspberry)
