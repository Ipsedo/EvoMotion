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
$ pacman -Syu nvidia cuda cmake bullet glm glew glfw imgui-full
```

#### LibTorch

Download `libtorch` from torch official website and then copy the extracted folder in `/opt/` (here version 2.3.0 with
CUDA 12.1) :

```bash
$ # as root
$ unzip /path/to/libtorch-cxx11-abi-shared-with-deps-2.3.0+cu121.zip -d /opt/
```

### Docker

It needs `nvidia-container-toolkit` package.

```bash
$ # example on arch linux
$ sudo pacman -Sy nvidia-container-toolkit
$ sudo nvidia-ctk runtime configure --runtime=docker
$ sudo systemctl restart docker
```

Build and run image :

```bash
$ cd /path/to/EvoMotion
$ # build image
$ docker build . --tag evo_motion
$ # run training
$ docker run -v /path/to/your/local/output_folder:/opt/evo_motion/out_train_muscle_ppo --rm --runtime=nvidia --gpus all evo_motion robot_walk ppo --agent_parameters seed=1234 hidden_size=256 gamma=0.99 lambda=0.95 epsilon=0.2 epoch=8 batch_size=32 learning_rate=1e-3 replay_buffer_size=1024 train_every=8 entropy_factor=0.01 critic_loss_factor=0.5 grad_norm_clip=0.5 --cuda --env_seed 1234  train /opt/evo_motion/out_train_muscle_ppo --episodes 512 --nb_saves 4096
```

To run graphical view of your trained agent you need first :

```bash
$ xhost +local:docker
```

And then run the image :

```bash
$ docker run -e DISPLAY=$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix -v /path/to/your/local/output_folder:/opt/evo_motion/out_train_muscle_ppo --rm --runtime=nvidia --gpus all evo_motion robot_walk ppo --agent_parameters seed=1234 hidden_size=256 gamma=0.99 lambda=0.95 epsilon=0.2 epoch=8 batch_size=32 learning_rate=1e-3 replay_buffer_size=1024 train_every=8 entropy_factor=0.01 critic_loss_factor=0.5 grad_norm_clip=0.5 --cuda --env_seed 1234 run /opt/evo_motion/out_train_muscle_ppo/save_0 -w 1920 -h 1080
```

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

   Run training on creature muscles
   ```bash
   $ cd /path/to/EvoMotion/build
   $ evo_motion robot_walk ppo --agent_parameters seed=1234 hidden_size=256 gamma=0.99 lambda=0.95 epsilon=0.2 epoch=8 batch_size=32 learning_rate=1e-3 replay_buffer_size=1024 train_every=8 entropy_factor=0.01 critic_loss_factor=0.5 grad_norm_clip=0.5 --cuda --env_seed 1234 train ./out/robot_walk_ppo --episodes 512 --nb_saves 4096
   ```
4. After the first save (here after 1024 episodes), you can now evaluate your trained agent.

   Evaluate agent on creature muscles (here the first model save) with GLFW window of 1920 * 1024 pixels
   ```bash
   $ evo_motion robot_walk ppo --agent_parameters seed=1234 hidden_size=256 gamma=0.99 lambda=0.95 epsilon=0.2 epoch=8 batch_size=32 learning_rate=1e-3 replay_buffer_size=1024 train_every=8 entropy_factor=0.01 critic_loss_factor=0.5 grad_norm_clip=0.5 --cuda --env_seed 1234 run ./out/robot_walk_ppo/save_0 -w 1920 -h 1024
   ```

## References

[1] Reinforcement Learning: An Introduction - *Richard S. Sutton and Andrew G. Barto* - Second Edition 2018

[2] Liquid Time-constant Networks - *Ramin Hasani, Mathias Lechner, Alexander Amini, Daniela Rus, Radu Grosu* - 8 Jun 2020

[4] Soft Actor-Critic Algorithms and Applications - *Tuomas Haarnoja, Aurick Zhou, Kristian Hartikainen, George Tucker, Sehoon Ha, Jie Tan, Vikash Kumar, Henry Zhu, Abhishek Gupta, Pieter Abbeel, Sergey Levine* - 29 Jan 2019

[3] Proximal Policy Optimization Algorithms - *John Schulman, Filip Wolski, Prafulla Dhariwal, Alec Radford, Oleg Klimov* - 28 Aug 2017
