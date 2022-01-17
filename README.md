# ALDA-Analysis

Creating Concise and Efficient Dynamic Analyses with ALDA.

## Introduction

This project implements a DSL parser & runtime library for dynamic analysis named ALDA(A novel Language for Dynamic Analyses). It now supports eraser, fast track, memory sanitizer, use-after-free analyses. 

## Artifact Evaluation

+ Virtual Machine: [DOI](https://doi.org/10.5281/zenodo.5748338). Username: **anony** Password: **password**
+ Document: [AE](AE.pdf)

## Project Structures

+ G-parser: The project for parsing DSL & generate analysis c++ code
+ rtLib: The runtime library for built-in data structures
+ pass: The LLVM opt pass to do instrumentation

## Environment Requirements

+ LLVM 6.0 (If you are using newer version, you might need to make slight changes on [CMakeLists.txt](eraser/CMakeLists.txt))
+ flex & bison
+ [C++ boost library](https://www.boost.org/)
+ Python 3.5+
+ [Python fire](https://github.com/google/python-fire)
+ [Python Cheetah3](https://pythonhosted.org/Cheetah/)

**fire & Cheetah3 could be installed through pip3**

## Steps to Run 

+ Please check artifact evaluation document: [AE](AE.pdf)

## Full Syntax

+ Please check the full syntax here: [Syntax](ALDA_Full_Syntax.pdf)


