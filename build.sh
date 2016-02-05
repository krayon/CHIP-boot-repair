#!/bin/bash

rm -rf build
mkdir build
cd build
cmake -DCMAKE_TOOLCHAIN_FILE=Linux-i686.Toolchain.cmake ..
make
cpack
