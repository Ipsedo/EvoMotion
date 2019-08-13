//
// Created by samuel on 11/08/19.
//

#ifndef EVOMOTION_DEEP_Q_LEARNING_H
#define EVOMOTION_DEEP_Q_LEARNING_H

#include <torch/torch.h>
#include <random>
#include "agent.h"

struct q_network : torch::nn::Module {

    q_network(torch::IntArrayRef state_space, torch::IntArrayRef action_space);
    torch::Tensor forward(torch::Tensor input);

    torch::nn::Linear l1{nullptr};
    torch::nn::Linear l2{nullptr};

};

// https://github.com/udacity/deep-reinforcement-learning/blob/master/dqn/solution/dqn_agent.py
struct dqn_agent : agent {

    // Random
    std::default_random_engine rd_gen;
    std::uniform_real_distribution<float> rd_uni;

    // LibTorch stuff
    q_network local_q_network;
    q_network target_q_network;
    torch::optim::Adam optimizer;

    // Algo hyper-params
    int batch_size;
    float gamma;
    float tau;
    int update_every;
    int idx_step;

    dqn_agent(torch::IntArrayRef state_space, torch::IntArrayRef action_space);
    void step(torch::Tensor state, torch::Tensor action, float reward,
              torch::Tensor next_state, bool done) override;
    torch::Tensor act(torch::Tensor state, float eps) override;

    void learn(torch::Tensor states, torch::Tensor actions, torch::Tensor rewards,
               torch::Tensor next_states, torch::Tensor dones);

    void soft_update();
};

#endif //EVOMOTION_DEEP_Q_LEARNING_H
