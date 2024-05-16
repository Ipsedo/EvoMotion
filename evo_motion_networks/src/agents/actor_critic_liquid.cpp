//
// Created by samuel on 29/03/24.
//

#include "./actor_critic_liquid.h"

#include "../init.h"

/*
 * Liquid Cell
 */

LiquidCellModule::LiquidCellModule(
    const std::vector<int64_t> &state_space, int neuron_number, const int unfolding_steps) {
    this->neuron_number = neuron_number;
    steps = unfolding_steps;

    weight = register_module(
        "weight",
        torch::nn::Sequential(
            torch::nn::Linear(state_space[0], state_space[0] * 2), torch::nn::Mish(),
            torch::nn::LayerNorm(torch::nn::LayerNormOptions({state_space[0] * 2})
                                     .elementwise_affine(true)
                                     .eps(1e-5f)),
            torch::nn::Linear(state_space[0] * 2, state_space[0] * 2), torch::nn::Mish(),
            torch::nn::LayerNorm(torch::nn::LayerNormOptions({state_space[0] * 2})
                                     .elementwise_affine(true)
                                     .eps(1e-5f)),
            torch::nn::Linear(
                torch::nn::LinearOptions(state_space[0] * 2, neuron_number).bias(false))));

    recurrent_weight = register_module(
        "recurrent_weight",
        torch::nn::Linear(torch::nn::LinearOptions(neuron_number, neuron_number).bias(false)));

    bias = register_parameter("bias", torch::randn({1, neuron_number}));

    a = register_parameter("a", torch::ones({1, neuron_number}));
    tau = register_parameter("tau", torch::ones({1, neuron_number}));

    reset_x_t();
}

void LiquidCellModule::reset_x_t() {/
    x_t = torch::zeros({1, neuron_number}, torch::TensorOptions().device(recurrent_weight->weight.device()));
}

torch::Tensor
LiquidCellModule::compute_step(const torch::Tensor &x_t_curr, const torch::Tensor &i_t) {
    return torch::mish(weight->forward(i_t) + recurrent_weight->forward(x_t_curr) + bias);
}
torch::Tensor LiquidCellModule::forward(const torch::Tensor &state) {
    const float delta_t = 1.f / static_cast<float>(steps);

    for (int i = 0; i < steps; i++)
        x_t = (x_t + delta_t * compute_step(x_t, state) * a)
              / (1.f + delta_t * (1.f / tau + compute_step(x_t, state)));

    return x_t;
}

void LiquidCellModule::to(torch::Device device, const bool non_blocking) {
    Module::to(device, non_blocking);
    x_t = x_t.to(device, non_blocking);
}

/*
 * Actor critic
 */

// actor

ActorLiquidNetwork::ActorLiquidNetwork(
    const std::vector<int64_t> &state_space, std::vector<int64_t> action_space,
    const int hidden_size, const int unfolding_steps) {

    liquid_network = register_module(
        "liquid_network",
        std::make_shared<LiquidCellModule>(state_space, hidden_size, unfolding_steps));

    mu = register_module(
        "mu", torch::nn::Sequential(
                  torch::nn::Linear(hidden_size, hidden_size * 2), torch::nn::Mish(),
            torch::nn::LayerNorm(
                torch::nn::LayerNormOptions({hidden_size * 2}).elementwise_affine(true).eps(1e-5f)),

            torch::nn::Linear(hidden_size * 2, hidden_size * 2), torch::nn::Mish(),
            torch::nn::LayerNorm(
                torch::nn::LayerNormOptions({hidden_size * 2}).elementwise_affine(true).eps(1e-5f)),

            torch::nn::Linear(hidden_size * 2, action_space[0]), torch::nn::Tanh()));

    sigma = register_module(
        "sigma", torch::nn::Sequential(
                     torch::nn::Linear(hidden_size, hidden_size * 2), torch::nn::Mish(),
                     torch::nn::LayerNorm(torch::nn::LayerNormOptions({hidden_size * 2})
                                     .elementwise_affine(true).eps(1e-5f)),

            torch::nn::Linear(hidden_size * 2, hidden_size * 2), torch::nn::Mish(),
            torch::nn::LayerNorm(
                torch::nn::LayerNormOptions({hidden_size * 2}).elementwise_affine(true).eps(1e-5f)),

            torch::nn::Linear(hidden_size * 2, action_space[0]), torch::nn::Softplus()));

    this->apply(init_weights);
}

actor_response ActorLiquidNetwork::forward(const torch::Tensor &state) {
    const auto x_t = liquid_network->forward(state.unsqueeze(0));

    return {mu->forward(x_t).squeeze(0), sigma->forward(x_t).squeeze(0)};
}

void ActorLiquidNetwork::reset_liquid() const { liquid_network->reset_x_t(); }

// critic

CriticLiquidNetwork::CriticLiquidNetwork(
    const std::vector<int64_t> &state_space, int hidden_size, int unfolding_steps) {
    liquid_network = register_module(
        "liquid_network",
        std::make_shared<LiquidCellModule>(state_space, hidden_size, unfolding_steps));

    critic = register_module(
        "critic", torch::nn::Sequential(
                      torch::nn::Linear(hidden_size, hidden_size * 2), torch::nn::Mish(),
                      torch::nn::LayerNorm(torch::nn::LayerNormOptions({hidden_size * 2})
                                     .elementwise_affine(true)
                                     .eps(1e-5f)),

                      torch::nn::Linear(hidden_size * 2, hidden_size * 2), torch::nn::Mish(),
            torch::nn::LayerNorm(
                torch::nn::LayerNormOptions({hidden_size * 2}).elementwise_affine(true).eps(1e-5f)),

            torch::nn::Linear(hidden_size * 2, 1)));

    this->apply(init_weights);
}

critic_response CriticLiquidNetwork::forward(const torch::Tensor &state) {
    const auto x_t = liquid_network->forward(state.unsqueeze(0));
    return {critic->forward(x_t).squeeze(0)};
}

void CriticLiquidNetwork::reset_liquid() const { liquid_network->reset_x_t(); }

/*
 * Agent
 */

ActorCriticLiquid::ActorCriticLiquid(
    const int seed, const std::vector<int64_t> &state_space,
    const std::vector<int64_t> &action_space, int hidden_size, float lr)
    : ActorCritic(seed, state_space, action_space, hidden_size, lr) {

    actor = std::make_shared<ActorLiquidNetwork>(state_space, action_space, hidden_size, 6);
    actor_optimizer = std::make_shared<torch::optim::Adam>(actor->parameters(), lr);

    critic = std::make_shared<CriticLiquidNetwork>(state_space, hidden_size, 6);
    critic_optimizer = std::make_shared<torch::optim::Adam>(critic->parameters(), lr);

    actor_loss_factor = 1.f;
    critic_loss_factor = 1.f;
}

void ActorCriticLiquid::done(const float reward) {
    ActorCritic::done(reward);

    std::dynamic_pointer_cast<ActorLiquidNetwork>(actor)->reset_liquid();
    std::dynamic_pointer_cast<CriticLiquidNetwork>(critic)->reset_liquid();
}