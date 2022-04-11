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

+ LLVM 13.0.1 (If you are using newer version, you might need to make slight changes on [CMakeLists.txt](eraser/CMakeLists.txt))
+ flex & bison
+ [C++ boost library](https://www.boost.org/)
+ Python 3.5+
+ [Python fire](https://github.com/google/python-fire)
+ [Python Cheetah3](https://pythonhosted.org/Cheetah/)

**fire & Cheetah3 could be installed through pip3**

## Steps to Run

### LLVM-13 Special notions

To rerun the experiments in ALDA paper, please check for following steps to resolve errors:

1. Remove all the **-Werror** in the compiler flag. Because the source inputs are from LLVM-6.0(in the AE), we need to ignore the 

    warning: overriding the module target triple with x86_64-unknown-linux-gnu [-Woverride-module]
3. Add **-enable-new-pm=0** for all **opt** commands to ensure the opt find our pass

+ Please check artifact evaluation document: [AE](AE.pdf)

## Full Syntax

+ Please check the full syntax here: [Syntax](ALDA_Full_Syntax.pdf)


