# EvoMotion

__author__ : Samuel Berrien

## Description

The final goal of this project is to implement any kind of optimisation algorithm in order to teach creature to walk,
run etc.

## Requirements

This project requires multiple libraries.

* physic engine stuff : Bullet
* graphic stuff : OpenGL, GLM, GLEW, GLFW
* reinforcement learning : LibTorch CXX 11 ABI (not pre-cxx 11 ABI) according to your CUDA version. Put `libtorch`
  folder in `/opt/`, (see cmake files)
* utils
  stuff : [p-ranav argparse](https://github.com/p-ranav/argparse), [p-ranav indicators](https://github.com/p-ranav/indicators), [nlohmann json](https://github.com/nlohmann/json)

### ArchLinux

Install dependencies with `pacman` :

```bash
$ # as root
$ pacman -Syu cmake bullet glm glew glfw
```

Download `libtorch` from torch official website and then copy the extracted folder in `/opt/` (here version 2.3.0 with
CUDA 12.1) :

```bash
$ # as root
$ unzip /path/to/libtorch-cxx11-abi-shared-with-deps-2.3.0+cu121.zip -d /opt/
```

### Docker

need `nvidia-container-toolkit` package.

```bash
$ # example on arch linux
$ sudo pacman -Sy nvidia-container-toolki
$ sudo nvidia-ctk runtime configure --runtime=docker
$ sudo systemctl restart docker
```

Build and run image :

```bash
$ cd /path/to/EvoMotion
$ # build image
$ docker build . --tag evo_motion
$ # run training
$  docker run --rm --runtime=nvidia --gpus all evo_motion muscles actor_critic_liquid --seed 12345 --cuda --hidden_size 32 train /path/to/out_muscle_a2c_liquid --episodes 512 --nb_saves 4096 --learning_rate 1e-3
```

To run graphical view of your trained agent you need first :

```bash
$ xhost +local:docker
```

And then run the image :

```bash
$ docker run -e DISPLAY=$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix --rm --runtime=nvidia --gpus all evo_motion muscles actor_critic_liquid --seed 30543 --hidden_size 32 --cuda run /path/to/out_muscle_a2c_liquid/save_0 -w 1920 -h 1080
```

__/!\ Volume needs to be bind for training and running or model won't be found /!\__

### Other OS

Any attempt to build this project on other OS will be appreciated to complete this section.

## Build and run

To build this project you need a decent C++ compiler with cmake and make.
An internet connexion is also required in order to download dependencies inside cmake file.

1. Clone the project
   ```bash
   $ git clone https://github.com/Ipsedo/EvoMotion.git
   ```
2. Then build the project:
    ```bash
    $ cd /path/to/EvoMotion
    $ mkdir build && cd build && cmake .. && make
    ```
3. Ready for training ! You can choose the environment on which train the agent you want.

   Run training on 3D cartpole
   ```bash
    $ cd /path/to/EvoMotion/build
    $ evo_motion cartpole3d actor_critic --seed 1234 --cuda --hidden_size 32 train ./out/cartpole3d_a2c --episodes 1024 --nb_saves 1024 --learning_rate 1e-3
   ```

   Run training on creature muscles
   ```bash
   $ cd /path/to/EvoMotion/build
   $ evo_motion muscles actor_critic_liquid --seed 1234 --cuda --hidden_size 32 train ./out/muscles_a2c_liquid --episodes 1024 --nb_saves 1024 --learning_rate 1e-3
   ```
4. After the first save (here after 1024 episodes), you can now evaluate your trained agent.

   Evaluate agent on 3D cartpole (here the first model save) with GLFW window of 1920 * 1024 pixels
   ```bash
   $ evo_motion cartpole3d actor_critic --seed 1234 --hidden_size 32 --cuda run ./out/cartpole3d_a2c/save_0 -w 1920 -h 1024
   ```

   Evaluate agent on creature muscles (here the first model save) with GLFW window of 1920 * 1024 pixels
   ```bash
   $ evo_motion muscles actor_critic_liquid --seed 1234 --hidden_size 32 --cuda run ./out/muscles_a2c_liquid/save_0 -w 1920 -h 1024
   ```

## References

[1] Reinforcement Learning: An Introduction - *Richard S. Sutton and Andrew G. Barto* - Second Edition 2018

[2] Liquid Time-constant Networks - *Ramin Hasani, Mathias Lechner, Alexander Amini, Daniela Rus, Radu Grosu* - 8 Jun
2020
