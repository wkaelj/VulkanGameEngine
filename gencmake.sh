#! /bin/bash

#create build folder
<<<<<<< Updated upstream
mkdir -p build
cp -r Config build/
=======
mkdir -p build/Config/shaders
#copy config files into build folder
cp Config/*.txt build/Config/*.txt
>>>>>>> Stashed changes

./compile_shaders.sh
#run cmake
cmake -S . -B build/