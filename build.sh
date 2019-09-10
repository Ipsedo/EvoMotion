#!/usr/bin/env bash
if [ -d "./build" ]; then
	rm -rf ./build
fi
mkdir build
cd build
cmake ..
make -j 8
cd ..