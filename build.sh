#!/bin/bash

cd build
cmake ..
make
cpack
