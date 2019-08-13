//
// Created by samuel on 11/08/19.
//

#ifndef EVOMOTION_DEEP_Q_LEARNING_H
#define EVOMOTION_DEEP_Q_LEARNING_H

#include <torch/torch.h>

struct q_network : torch::nn::Module {

    q_network(torch::IntArrayRef state_space, torch::IntArrayRef action_space);
    torch::Tensor forward(torch::Tensor input);

    torch::nn::Linear l1{nullptr};
    torch::nn::Linear l2{nullptr};

};




#endif //EVOMOTION_DEEP_Q_LEARNING_H
