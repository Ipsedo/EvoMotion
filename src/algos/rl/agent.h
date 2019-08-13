//
// Created by samuel on 13/08/19.
//

#ifndef EVOMOTION_AGENT_H
#define EVOMOTION_AGENT_H

#include <torch/torch.h>
#include <deque>
#include "deep_q_learning.h"

struct memory {
    torch::Tensor state;
    torch::Tensor action;
    float reward;
    torch::Tensor next_state;
    bool done;
};

struct replay_buffer {
    std::deque<memory> mem;
    int max_size;

    explicit replay_buffer(int max_size);
    void add(torch::Tensor state, torch::Tensor action, float reward, torch::Tensor next_state, bool done);
    std::tuple<torch::Tensor, torch::Tensor, torch::Tensor, torch::Tensor, torch::Tensor>
        sample(int batch_size);
};

// https://github.com/udacity/deep-reinforcement-learning/blob/master/dqn/solution/dqn_agent.py
struct agent {
    torch::IntArrayRef m_state_space, m_action_space;
    replay_buffer memory_buffer;

    agent(torch::IntArrayRef state_space, torch::IntArrayRef action_space, int buffer_size);
    virtual void step(torch::Tensor state, torch::Tensor action, float reward, torch::Tensor next_state, bool done) = 0;
    virtual torch::Tensor act(torch::Tensor state, float eps) = 0;
};

struct random_agent : agent {
    random_agent(torch::IntArrayRef state_space, torch::IntArrayRef action_space);
    void step(torch::Tensor state, torch::Tensor action, float reward, torch::Tensor next_state, bool done) override;
    torch::Tensor act(torch::Tensor state, float eps) override;
};

struct dqn_agent : agent {
    q_network local_q_network;
    q_network target_q_network;
    torch::optim::Adam optimizer;

    int batch_size = 64;
    float gamma = 0.9f;
    float tau = 1e-3f;
    int update_every = 4;

    int idx_step;

    dqn_agent(torch::IntArrayRef state_space, torch::IntArrayRef action_space);
    void step(torch::Tensor state, torch::Tensor action, float reward,
            torch::Tensor next_state, bool done) override;
    torch::Tensor act(torch::Tensor state, float eps) override;

    void learn(torch::Tensor states, torch::Tensor actions, torch::Tensor rewards,
            torch::Tensor next_states, torch::Tensor dones);

    void soft_update();
};

#endif //EVOMOTION_AGENT_H
