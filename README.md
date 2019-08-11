# EvoMotion
__author__ : Samuel Berrien

## Description
The final goal of this project is to implement CMA-ES or any kind of optimisation algorithm in order to teach creature to walk, run etc.

We will first try to reproduce the results of the article [Flexible Muscle-Based Locomotion for Bipedal Creatures](https://www.goatstream.com/research/papers/SA2013/SA2013.pdf).

__Note__ : In a first time, only CMA-ES will be implemented and tested with [COCO](https://github.com/numbbo/coco) benchmark tools. Then the creature walk teaching will be implemented.

## Requirements
This project requires multiple libraries.

Linear algebra stuff : 
* LAPACK
* BLAS
* Boost
* Armadillo

Physic engine stuff :
* Bullet

Graphic stuff :
* OpenGL
* GLM
* GLEW
* GLFW
* SOIL

### Ubuntu
LAPACK, BLAS and Boost :
```bash
$ # Update
$ sudo apt-get update
$ # Install LAPACK
$ sudo apt-get install liblapack-dev
$ # Install BLAS
$ sudo apt-get install libblas-dev
$ # Install Boost
$ sudo apt-get install libboost-dev
```
Armadillo and Bullet :
```bash
$ # Update
$ sudo apt-get update
$ # Install Armadillo
$ sudo apt-get install libarmadillo-dev
$ # Install Bullet3
$ sudo apt-get install libbullet-dev
```
GLM, GLEW and GLFW :
```bash
$ # Update
$ sudo apt-get update
$ # Install GLM
$ sudo apt-get install libglm-dev
$ # Install GLEW
$ sudo apt-get install libglew-dev
$ # Install GLFW-3
$ sudo apt-get install libglfw3-dev
```
SOIL :
```bash
$ # Update
$ sudo apt-get update
$ # Install SOIL
$ sudo apt-get install libsoil-dev
```

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


