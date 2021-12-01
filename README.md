# ALDA-Analysis

Creating Concise and Efficient Dynamic Analyses with ALDA.

## Introduction

This project implements a DSL parser & runtime library for dynamic analysis named ALDA(A novel Language for Dynamic Analyses). It now supports eraser, fast track, memory sanitizer, use-after-free analyses. 

## Artifact Evaluation

+ Virtual Machine: 
+ Document: 

## Project Structures

+ G-parser: The project for parsing DSL & generate analysis cpp code
+ rtLib: The runtime library for built-in data structures
+ pass: The LLVM opt pass to do instrumentation

## Environment Requirements

+ LLVM 6.0 (If you are using other version, you might need to make slight changes on [CMakeLists.txt](eraser/CMakeLists.txt))
+ flex & bison
+ [C++ boost library](https://www.boost.org/)
+ Python 3.5+
+ [Python fire](https://github.com/google/python-fire)
+ [Python Cheetah3](https://pythonhosted.org/Cheetah/)

**fire & Cheetah3 could be installed through pip3**

## Steps to Run 

1. Build your analysis(e.g. memory sanitizer):

        ./auto_run.sh ./msan 0

2. Link analysis with your target program

        llvm-link target.bc rtLib.bc -o combined.bc

3. Do instrumentation

        # Replace the files with their absolute path
        opt -load eraser.so -eraser combined.bc -o inst.bc

4. Link the executable

        clang++ -O2 inst.bc -lpthread
