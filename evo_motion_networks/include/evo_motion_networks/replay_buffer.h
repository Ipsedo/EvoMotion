//
// Created by samuel on 11/11/24.
//

#ifndef EVO_MOTION_REPLAY_BUFFER_H
#define EVO_MOTION_REPLAY_BUFFER_H

#include <random>

#include <torch/torch.h>

struct step_replay_buffer {
    torch::Tensor state;
    torch::Tensor action;
    float reward;
    bool done;
    torch::Tensor next_state;
};

struct liquid_step_memory {
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

struct liquid_step_replay_buffer {
    torch::Tensor state;
    liquid_step_memory x_t;
    torch::Tensor action;
    float reward;
    bool done;
    torch::Tensor next_state;
    liquid_step_memory next_x_t;
};

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

class ReplayBuffer : public AbstractReplayBuffer<step_replay_buffer, float, torch::Tensor, bool> {
public:
    ReplayBuffer(int size, int seed);

    //void update_last(float reward, torch::Tensor next_state, bool done) override;

protected:
    step_replay_buffer update_last_item(
        step_replay_buffer last_item, float reward, torch::Tensor next_state, bool done) override;
};

class LiquidReplayBuffer
    : public AbstractReplayBuffer<liquid_step_replay_buffer, float, torch::Tensor, bool> {
public:
    LiquidReplayBuffer(int size, int seed);
    //void update_last(float reward, torch::Tensor next_state, bool done) override;
protected:
    liquid_step_replay_buffer update_last_item(
        liquid_step_replay_buffer last_item, float reward, torch::Tensor next_state,
        bool done) override;
};

#endif//EVO_MOTION_REPLAY_BUFFER_H
