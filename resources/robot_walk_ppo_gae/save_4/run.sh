#!/usr/bin/env bash

evo_motion robot_walk ppo --agent_parameters hidden_size=512 seed=1234 learning_rate=1e-4 batch_size=16 gamma=0.99 epsilon=0.2 lambda=0.95 epoch=8 critic_loss_factor=0.5 entropy_factor=0.01 clip_grad_norm=0.5 --env_seed 1234 --cuda run ./resources/robot_walk_ppo_gae/save_4 -w 1920 -h 1080