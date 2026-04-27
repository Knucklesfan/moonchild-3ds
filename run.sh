#!/bin/bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
cp ./build/moonchild ./assets/moonchild/
cd assets/moonchild/
./moonchild
cd ../../