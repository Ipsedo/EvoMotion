# EvoMotion

__author__ : Samuel Berrien

## Description

The final goal of this project is to implement any kind of optimisation algorithm in order to teach creature to walk,
run etc.

We will first try to reproduce the results of the
article [Flexible Muscle-Based Locomotion for Bipedal Creatures](https://www.goatstream.com/research/papers/SA2013/SA2013.pdf).

## Build and run

To build this project you need a decent C++ compiler with cmake and make.
An internet connexion is also required in order to download dependencies inside cmake file.

1. Clone the project
2. Then build the project:
    ```bash
    $ cd /path/to/EvoMotion
    $ mkdir build && cd build
    $ cmake ..
    $ make
    ```
3. Run training on 3D cartpole
    ```bash
   $ cd /path/to/EvoMotion/build
   $ evo_motion cartpole3d --seed 1234 --cuda --hidden_size 32 train ./out/cartpole3d_a2c --episodes 1024 --nb_saves 1024 --learning_rate 1e-3
   ```
4. Evaluate agent (here the 20th model save) with GLFW window of 1920 * 1024 pixels
   ```bash
   $ evo_motion cartpole3d --seed 1234 --hidden_size 32 --cuda run ./out/cartpole3d_a2c/save_20 -w 1920 -h 1024
   ```

## Requirements

This project requires multiple libraries.

* physic engine stuff : Bullet
* graphic stuff : OpenGL, GLM, GLEW, GLFW
* reinforcement learning : LibTorch CXX 11 ABI, not pre-cxx 11 ABI. Put `libtorch` folder in `/opt/`, (see cmake files)
* utils stuff : [argparse](https://github.com/p-ranav/argparse), [indicators](https://github.com/p-ranav/indicators)

## References

[1] Reinforcement Learning: An Introduction - *Richard S. Sutton and Andrew G. Barto* - Second Edition 2018
