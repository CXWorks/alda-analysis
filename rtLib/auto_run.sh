#!/usr/bin/env bash
clang++ -O2 -emit-llvm -o rtLib.bc -c cxx_gen.cpp && mv rtLib.bc ../
