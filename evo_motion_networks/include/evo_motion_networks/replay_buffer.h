//
// Created by samuel on 11/11/24.
//

#ifndef EVO_MOTION_REPLAY_BUFFER_H
#define EVO_MOTION_REPLAY_BUFFER_H

#include <random>

#include <torch/torch.h>

// step structs

struct episode_step {
    torch::Tensor state;
    torch::Tensor action;
    float reward;
    bool done;
    torch::Tensor next_state;
};

struct liquid_a2c_step_memory {
    torch::Tensor actor_x_t;
    torch::Tensor critic_x_t;
};

struct liquid_sac_step_memory {
    torch::Tensor actor_x_t;
    torch::Tensor critic_1_x_t;
    torch::Tensor critic_2_x_t;
    torch::Tensor target_critic_1_x_t;
    torch::Tensor target_critic_2_x_t;
};

struct liquid_a2c_episode_step {
    episode_step replay_buffer;
    liquid_a2c_step_memory x_t;
    liquid_a2c_step_memory next_x_t;
};

struct liquid_sac_episode_step {
    episode_step replay_buffer;
    liquid_sac_step_memory x_t;
    liquid_sac_step_memory next_x_t;
};

// Replay buffer classes

template<typename ReplayBufferType, class... UpdateArgs>
class AbstractReplayBuffer {
public:
    AbstractReplayBuffer(int size, int seed);
    virtual std::vector<ReplayBufferType> sample(int batch_size);
    virtual void add(ReplayBufferType buffer);
    virtual void update_last(UpdateArgs... args);
    virtual bool empty();

protected:
    virtual ReplayBufferType update_last_item(ReplayBufferType last_item, UpdateArgs... args) = 0;

private:
    int size;
    std::vector<ReplayBufferType> memory;
    std::mt19937 rand_gen;
};

class ReplayBuffer : public AbstractReplayBuffer<episode_step, float, torch::Tensor, bool> {
public:
    ReplayBuffer(int size, int seed);

protected:
    episode_step update_last_item(
        episode_step last_item, float reward, torch::Tensor next_state, bool done) override;
};

class LiquidA2cReplayBuffer
    : public AbstractReplayBuffer<liquid_a2c_episode_step, float, torch::Tensor, bool> {
public:
    LiquidA2cReplayBuffer(int size, int seed);

protected:
    liquid_a2c_episode_step update_last_item(
        liquid_a2c_episode_step last_item, float reward, torch::Tensor next_state,
        bool done) override;
};

class LiquidSacReplayBuffer
    : public AbstractReplayBuffer<liquid_sac_episode_step, float, torch::Tensor, bool> {
public:
    LiquidSacReplayBuffer(int size, int seed);

protected:
    liquid_sac_episode_step update_last_item(
        liquid_sac_episode_step last_item, float reward, torch::Tensor next_state,
        bool done) override;
};

#endif//EVO_MOTION_REPLAY_BUFFER_H
