#! /bin/bash

#create build folder
mkdir -p build/Config/shaders

./compile_shaders.sh
#run cmake
cmake -S . -B build/