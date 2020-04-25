# EvoMotion
__author__ : Samuel Berrien

## Description
The final goal of this project is to implement any kind of optimisation algorithm in order to teach creature to walk, run etc.

We will first try to reproduce the results of the article [Flexible Muscle-Based Locomotion for Bipedal Creatures](https://www.goatstream.com/research/papers/SA2013/SA2013.pdf).

### Done
* CMA-ES and [COCO](https://github.com/numbbo/coco) benchmark work well. But is removed from this project (moved to `EvoMotion/old`)
* RL environments with bullet and OpenGL/GLFW can be created (only cartpole is done for the moment)
* DQN and DDPG agents with libtorch are implemented and functional


## Requirements
This project requires multiple libraries.

* Physic engine stuff : Bullet

* Graphic stuff : OpenGL, GLM, GLEW, GLFW and SOIL

* Reinforcement Learning : LibTorch

### Ubuntu
Install deb dependencies :
```bash
$ # Update
$ sudo apt-get update
$ # Install libraries
$ sudo apt-get install libbullet-dev libglm-dev libglew-dev libglfw3-dev libsoil-dev
```
Install LibTorch :
```bash
$ # Download libtorch for CUDA 10.0
$ wget https://download.pytorch.org/libtorch/cu100/libtorch-shared-with-deps-1.2.0.zip
$ # Unzip libtorch to /opt
$ sudo unzip libtorch-shared-with-deps-latest.zip -d /opt
```
__Note__ : if you want to extract LibTorch in a custom directory you need to modify EvoMotion's `CMakeLists.txt`.

Install CLI11 :
```bash
$ # Clone CLI11
$ git clone --single-branch --branch v1.5 https://github.com/CLIUtils/CLI11.git
$ # Copy CLI11 headers to /usr/local/include
$ sudo cp -r CLI11/include/CLI /usr/local/include
```
__Note__ : if you want to place CLI11 headers in a different folder, just add `include_directories("/path/to/CLI11/include")` to `CMakeLists.txt`.

## Installation
First clone this repository :
```bash
$ git clone https://github.com/Ipsedo/EvoMotion.git
```

Finally you are able to build the source
```bash
$ # Go to EvoMotion root directory
$ cd EvoMotion
$ # Build project with CMake and Makefile
$ ./build.sh
```

## Usage
Pass the following arguments to programin order to launch specific test :
* `opengl` : test the graphical part of this project
* `bullet` : test the physical engine
* `rl` : test the CartPole environment with RL agent

Below the commands to start this project :
```bash
$ # Go to EvoMotion root dir
$ cd EvoMotion
$ # Build the project
$ ./build.sh
$ # Start Reinforcement Learning test
$ ./build/EvoMotion rl
```


