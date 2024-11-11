//
// Created by samuel on 11/11/24.
//

#include <evo_motion_networks/networks/critic.h>
#include <evo_motion_networks/init.h>

CriticModule::CriticModule(std::vector<int64_t> state_space, int hidden_size) {
    critic = register_module(
        "critic",
        torch::nn::Sequential(
            torch::nn::Linear(state_space[0], hidden_size), torch::nn::Mish(),
            torch::nn::LayerNorm(
                torch::nn::LayerNormOptions({hidden_size}).elementwise_affine(true).eps(1e-5)),

            torch::nn::Linear(hidden_size, hidden_size), torch::nn::Mish(),
            torch::nn::LayerNorm(
                torch::nn::LayerNormOptions({hidden_size}).elementwise_affine(true).eps(1e-5)),
            torch::nn::Linear(hidden_size, hidden_size), torch::nn::Mish(),
            torch::nn::LayerNorm(
                torch::nn::LayerNormOptions({hidden_size}).elementwise_affine(true).eps(1e-5)),

            torch::nn::Linear(hidden_size, 1)));

    critic->apply(init_weights);
}

critic_response CriticModule::forward(const torch::Tensor &state) {
    bool only_one = false;
    torch::Tensor in_critic;
    if (state.sizes().size() == 1) {
        in_critic = state.unsqueeze(0);
        only_one = true;
    } else in_critic = state;

    auto out_critic = critic->forward(in_critic);

    if (only_one) { out_critic = out_critic.squeeze(0); }
    return {out_critic};
}