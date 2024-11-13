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

struct liquid_a2c_memory {
    torch::Tensor actor_x_t;
    torch::Tensor critic_x_t;
};

struct liquid_sac_memory {
    torch::Tensor actor_x_t;
    torch::Tensor critic_1_x_t;
    torch::Tensor critic_2_x_t;
    torch::Tensor target_critic_1_x_t;
    torch::Tensor target_critic_2_x_t;
};

template<typename LiquidMemory>
struct liquid_episode_step {
    episode_step replay_buffer;
    LiquidMemory x_t;
    LiquidMemory next_x_t;
};

/*
 * Replay buffer classes
 */

// Abstract

template<class ReplayBufferType, class... UpdateArgs>
class AbstractReplayBuffer {
public:
    AbstractReplayBuffer(int size, int seed);
    virtual std::vector<ReplayBufferType> sample(int batch_size);
    virtual void add(ReplayBufferType buffer);
    virtual void update_last(UpdateArgs... args);
    virtual bool empty();

    virtual ~AbstractReplayBuffer();

protected:
    virtual ReplayBufferType update_last_item(ReplayBufferType last_item, UpdateArgs... args) = 0;

private:
    int size;
    std::vector<ReplayBufferType> memory;
    std::mt19937 rand_gen;
};

// replay buffer classes

class ReplayBuffer : public AbstractReplayBuffer<episode_step, float, torch::Tensor, bool> {
public:
    ReplayBuffer(int size, int seed);

protected:
    episode_step update_last_item(
        episode_step last_item, float reward, torch::Tensor next_state, bool done) override;
};

template<typename LiquidMemory>
class LiquidReplayBuffer
    : public AbstractReplayBuffer<liquid_episode_step<LiquidMemory>, float, torch::Tensor, bool> {
public:
    LiquidReplayBuffer(int size, int seed);

protected:
    liquid_episode_step<LiquidMemory> update_last_item(
        liquid_episode_step<LiquidMemory> last_item, float reward, torch::Tensor next_state,
        bool done) override;
};

template class LiquidReplayBuffer<liquid_a2c_memory>;
template class LiquidReplayBuffer<liquid_sac_memory>;

#endif//EVO_MOTION_REPLAY_BUFFER_H
