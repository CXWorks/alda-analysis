cmake_minimum_required(VERSION 3.1)
project(G_parser)

set(CMAKE_CXX_STANDARD 17)
add_compile_options(-g)
set(CMAKE_CXX_COMPILER clang++-6.0)
set(CMAKE_C_COMPILER clang-6.0)
set(CMAKE_SOURCE_DIR /home/cxworks/software/boost_1_70_0)

include_directories(${CMAKE_SOURCE_DIR})
link_directories(${CMAKE_SOURCE_DIR}/stage/lib)

add_custom_target(
    flex_target
        COMMAND flex -o tokens.cpp ${CMAKE_CURRENT_SOURCE_DIR}/tokens.l
)

add_custom_target(
    bison_target
        COMMAND bison -o parser.cpp -d ${CMAKE_CURRENT_SOURCE_DIR}/parser.y
)


add_executable(G_parser statement.h generator.h parser.hpp parser.cpp tokens.cpp statement.cpp )
add_dependencies(G_parser bison_target)
add_dependencies(G_parser flex_target)


