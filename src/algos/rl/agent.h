//
// Created by samuel on 13/08/19.
//

#ifndef EVOMOTION_AGENT_H
#define EVOMOTION_AGENT_H

#include <torch/torch.h>
#include <deque>
#include <random>

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

    std::default_random_engine rd_gen;
    std::uniform_real_distribution<float> rd_uni;

    explicit replay_buffer(int max_size, unsigned long seed);
    void add(torch::Tensor state, torch::Tensor action, float reward, torch::Tensor next_state, bool done);
    std::tuple<torch::Tensor, torch::Tensor, torch::Tensor, torch::Tensor, torch::Tensor>
        sample(int batch_size);
};


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



#endif //EVOMOTION_AGENT_H
