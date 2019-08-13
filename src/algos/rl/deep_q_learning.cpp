//
// Created by samuel on 13/08/19.
//

#include "deep_q_learning.h"

q_network::q_network(torch::IntArrayRef state_space, torch::IntArrayRef action_space) {
    l1 = register_module("l1", torch::nn::Linear(state_space[0], 16));
    l2 = register_module("l2", torch::nn::Linear(16, action_space[0]));
}

torch::Tensor q_network::forward(torch::Tensor input) {
    auto out_l1 = torch::relu(l1->forward(input));
    auto pred = torch::softmax(l2->forward(out_l1), -1);
    return pred;
}



