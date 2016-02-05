#!/bin/bash

sudo apt-get -y update
sudo apt-get -y install g++ build-essential cmake libgtk2.0-dev libusb-1.0-0-dev

cmake --version
wget https://cmake.org/files/v3.4/cmake-3.4.3.tar.gz
tar xvzf cmake-3.4.3.tar.gz
cd cmake-3.4.3
./bootstrap && make && sudo make install
cd ..
