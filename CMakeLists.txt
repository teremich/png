cmake_minimum_required(VERSION 3.22)
project(mypng)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-g -fsanitize=address)
add_link_options(-fsanitize=address -flto)

add_library(mypng
src/chunk.cc
src/datastream.cc
src/file.cc
)
link_libraries(mypng z)
add_executable(test src/test.cc)
target_link_libraries(test PRIVATE mypng)
