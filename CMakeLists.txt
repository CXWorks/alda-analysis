cmake_minimum_required(VERSION 3.1)
project(auto-analysis)

set(CMAKE_CXX_STANDARD 17)
add_compile_options(-g)


add_subdirectory(G-parser)
add_subdirectory(pass)
add_subdirectory(rtLib)

