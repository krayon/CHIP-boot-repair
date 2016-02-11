#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

rm -rf build
mkdir build
cd build
cmake -DCMAKE_TOOLCHAIN_FILE=${DIR}/Linux-i686.Toolchain.cmake ..
make
cpack
