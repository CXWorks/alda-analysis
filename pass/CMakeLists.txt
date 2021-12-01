cmake_minimum_required(VERSION 3.10)
project(eraser)

find_package(LLVM 6.0 REQUIRED  PATH /usr/lib/llvm-6.0/)
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
if (${LLVM_PACKAGE_VERSION} STREQUAL "10.0.0")

    add_llvm_library(eraser MODULE opt_gen.cpp)
else ()


    add_llvm_loadable_module(eraser opt_gen.cpp)
endif ()