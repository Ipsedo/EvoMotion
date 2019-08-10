#!/usr/bin/env bash
echo "Build"
bash ./build.sh

rm -rf exdata/
echo "Removed exdata/"

rm -rf ppdata/
echo "Removed ppdata/"

echo "Start Algo"
./build/EvoMotion coco
python -m cocopp exdata/CMA_on_bbob
