cmake_minimum_required(VERSION 3.10)
project(eraser)

find_package(LLVM 13.0.1 REQUIRED  PATH  /usr/lib/llvm-13/)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
message(STATUS "LLVM_INCLUDE_DIRS=${LLVM_INCLUDE_DIRS}")
message(STATUS "LLVM_DEFINITIONS=${LLVM_DEFINITIONS}")
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})
message(STATUS "LLVM_DEFINITIONS=${LLVM_CMAKE_DIR}")
list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(AddLLVM)

set(CMAKE_CXX_STANDARD 17)

set(eraser opt_gen.cpp)

add_llvm_library(eraser MODULE opt_gen.cpp)
