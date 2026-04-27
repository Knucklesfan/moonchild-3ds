#!/bin/bash
make -j$(nproc) && azahar ./output/moonchild-3ds.3dsx
# cp ./build/moonchild ./assets/moonchild/
# cd assets/moonchild/
# ./moonchild
# cd ../../