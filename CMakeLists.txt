cmake_minimum_required(VERSION 3.22)
project(mypng)

add_library(mypng src/main.cc)
add_executable(test src/test.cc)
target_link_libraries(test PRIVATE mypng)
