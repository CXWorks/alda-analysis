cmake_minimum_required(VERSION 3.10)
project(rtLib)

set(CMAKE_CXX_STANDARD 14)
SET(GCC_COVERAGE_COMPILE_FLAGS "-fPIC")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")
SET(GCC_COVERAGE_LINK_FLAGS "-lpthread")
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}")

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
include_directories(${CMAKE_SOURCE_DIR})
add_library(rtLib rtLib.cpp rtLib.h util_bitmap_sparse_bitmap.h bit_vector.h)

add_library(rtLibGen cxx_gen.cpp rtLib.cpp rtLib.h PageMap.h util_bitmap_sparse_bitmap.h bit_vector.h)


