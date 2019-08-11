# EvoMotion
__author__ : Samuel Berrien

## Description
The final goal of this project is to implement CMA-ES or any kind of optimisation algorithm in order to teach creature to walk, run etc.

We will first try to reproduce the results of the article [Flexible Muscle-Based Locomotion for Bipedal Creatures](https://www.goatstream.com/research/papers/SA2013/SA2013.pdf).

__Note__ : In a first time, only CMA-ES will be implemented and tested with [COCO](https://github.com/numbbo/coco) benchmark tools. Then the creature walk teaching will be implemented.

## Requirements
This project requires multiple libraries.

* Linear algebra stuff : LAPACK, BLAS, Boost and Armadillo

* Physic engine stuff : Bullet

* Graphic stuff : OpenGL, GLM, GLEW, GLFW and SOIL

* Reinforcement Learning : LibTorch

### Ubuntu
```bash
$ # Update
$ sudo apt-get update
$ # Install libraries
$ sudo apt-get install liblapack-dev libblas-dev libboost-dev \
libarmadillo-dev libbullet-dev libglm-dev libglew-dev libglfw3-dev libsoil-dev
```
```bash
$ # Download libtorch for CUDA 10.0
$ wget https://download.pytorch.org/libtorch/nightly/cu100/libtorch-shared-with-deps-latest.zip
$ # Unzip libtorch to /opt
$ sudo unzip libtorch-shared-with-deps-latest.zip -d /opt
```
__Note__ : if you want to extract LibTorch in a custom directory you need to modify EvoMotion's `CMakeLists.txt`.

## Installation
First clone this repository :
```bash
$ git clone https://github.com/Ipsedo/EvoMotion.git
```

Then install coco benchmark platform :
```bash
$ # Go to EvoMotion root directory
$ cd EvoMotion
$ # Clone and install COCO
$ ./install_coco.sh
```

Finally you are able to build the source
```bash
$ # Go to EvoMotion root directory
$ cd EvoMotion
$ # Build project with CMake and Makefile
$ ./build.sh
```


