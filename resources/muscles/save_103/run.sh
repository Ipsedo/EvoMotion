#!/usr/bin/env bash

# run in current directory
evo_motion muscles actor_critic_liquid --env_seed 12345 --cuda -p hidden_size=32 -p seed=12345 -p batch_size=1 -p learning_rate=1e-3 -p gamma=1 -p unfolding_steps=6 -p entropy_factor=1e-2 run ./ -w 1920 -h 1080