#! /bin/bash

#create build folder
cp -r Config build/Config

mkdir -p build/Config/shaders

./compile_shaders.sh
#run cmake
cmake -S . -B build/