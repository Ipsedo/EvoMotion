//
// Created by samuel on 11/11/24.
//

#include <evo_motion_networks/init.h>
#include <evo_motion_networks/networks/q_net.h>

QNetworkModule::QNetworkModule(
    const std::vector<int64_t> &state_space, const std::vector<int64_t> &action_space,
    int hidden_size)
    : q_network(register_module(
          "q_network",
          torch::nn::Sequential(
              torch::nn::Linear(state_space[0] + action_space[0], hidden_size), torch::nn::Mish(),
              torch::nn::LayerNorm(
                  torch::nn::LayerNormOptions({hidden_size}).elementwise_affine(true).eps(1e-5)),

              torch::nn::Linear(hidden_size, hidden_size), torch::nn::Mish(),
              torch::nn::LayerNorm(
                  torch::nn::LayerNormOptions({hidden_size}).elementwise_affine(true).eps(1e-5)),
              torch::nn::Linear(hidden_size, hidden_size), torch::nn::Mish(),
              torch::nn::LayerNorm(
                  torch::nn::LayerNormOptions({hidden_size}).elementwise_affine(true).eps(1e-5)),

              torch::nn::Linear(hidden_size, 1)))) {

    apply(init_weights);
}

critic_response QNetworkModule::forward(const torch::Tensor &state, const torch::Tensor &action) {
    bool only_one = false;

    torch::Tensor in_q_net = torch::cat({state, action}, -1);

    if (in_q_net.sizes().size() == 1) {
        in_q_net = in_q_net.unsqueeze(0);
        only_one = true;
    } else in_q_net = in_q_net;

    auto q_value = q_network->forward(in_q_net);

    if (only_one) { q_value = q_value.squeeze(0); }
    return {q_value};
}