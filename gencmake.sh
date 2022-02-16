#! /bin/bash

#create build folder
mkdir -p build
cp -r Config build/

./compile_shaders.sh
#run cmake
cmake -S . -B build/