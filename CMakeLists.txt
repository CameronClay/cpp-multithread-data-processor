cmake_minimum_required(VERSION 3.13)

project(multithread_data_processor)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

enable_testing()

add_subdirectory(lib)
add_subdirectory(bench_utils)
add_subdirectory(data_processor)
add_subdirectory(tests)