#!/bin/bash

# This might work, or might not work.
git clone https://github.com/libsdl-org/SDL.git
cd SDL
git checkout SDL2
mkdir build && cd build
cmake -DCMAKE_TOOLCHAIN_FILE=$DEVKITPRO/cmake/3DS.cmake ..
make -j$(nproc)
sudo make install
cd ../../

git clone https://github.com/libsdl-org/SDL_mixer.git 
cd SDL_mixer
git checkout release-2.8.x
mkdir build && cd build
cmake -DCMAKE_TOOLCHAIN_FILE=$DEVKITPRO/cmake/3DS.cmake -DSDL2MIXER_MIDI=off ..
make -j$(nproc)
sudo make install
cd ../../