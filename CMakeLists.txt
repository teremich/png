cmake_minimum_required(VERSION 3.22)
project(mypng)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

add_compile_options(-g -fsanitize=address)
add_link_options(-fsanitize=address -flto)

add_library(mypng
    src/chunk.cc
    src/datastream.cc
    src/file.cc
    src/image.cc
    src/crc.cc
)
target_link_libraries(mypng PRIVATE z)
add_executable(test src/test.cc)
target_include_directories(test PRIVATE
    include/
    src/
)
target_link_directories(test PRIVATE lib)
target_link_libraries(test PRIVATE mypng bmp)
