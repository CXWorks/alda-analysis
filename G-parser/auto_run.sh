#!/usr/bin/env bash

cmake . && make clean && make

./G_parser $1.txt $2

mv *.json template/

cd template && python3 -u generate_cpp.py gen_cxx_cpp --instr=cxx.json --tlp=cxx_rtlib_template.ctt && cd ..

cd template && python3 -u generate_opt.py gen-opt-cpp --instr=opt.json --tlp=llvm_opt_template.ctt && cd ..

# mv *_gen.cpp ../
