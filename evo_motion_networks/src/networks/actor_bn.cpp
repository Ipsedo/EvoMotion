//
// Created by samuel on 09/01/25.
//

#include <evo_motion_networks/init.h>
#include <evo_motion_networks/networks/actor.h>
#include <evo_motion_networks/networks/norm.h>

BatchNormActorModule::BatchNormActorModule(
    std::vector<int64_t> state_space, std::vector<int64_t> action_space, int hidden_size)
    : head(register_module(
          "head", torch::nn::Sequential(
                      torch::nn::BatchNorm1d(static_cast<int>(state_space[0])),

                      torch::nn::Linear(state_space[0], hidden_size), torch::nn::Mish(),
                      torch::nn::BatchNorm1d(hidden_size),

                      torch::nn::Linear(hidden_size, hidden_size), torch::nn::Mish(),
                      torch::nn::BatchNorm1d(hidden_size)))),
      mu(register_module(
          "mu", torch::nn::Sequential(
                    torch::nn::Linear(hidden_size, action_space[0]), torch::nn::Tanh()))),
      sigma(register_module(
          "sigma", torch::nn::Sequential(
                       torch::nn::Linear(hidden_size, action_space[0]), torch::nn::Softplus()))) {

    apply(init_weights);
}

actor_response BatchNormActorModule::forward(const torch::Tensor &state) {
    bool only_one = false;
    torch::Tensor input_head;
    if (state.sizes().size() == 1) {
        input_head = state.unsqueeze(0);
        only_one = true;
    } else input_head = state;

    auto head_out = head->forward(input_head);
    auto out_mu = mu->forward(head_out);
    auto out_sigma = sigma->forward(head_out);

    if (only_one) {
        out_mu = out_mu.squeeze(0);
        out_sigma = out_sigma.squeeze(0);
    }

    return {out_mu, out_sigma};
}