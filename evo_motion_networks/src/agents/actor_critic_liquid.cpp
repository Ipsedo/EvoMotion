//
// Created by samuel on 29/03/24.
//

#include "./actor_critic_liquid.h"

#include "../init.h"

/*
 * Liquid Cell
 */

LiquidCellModule::LiquidCellModule(
    const int &input_space, int neuron_number, const int unfolding_steps) {
    this->neuron_number = neuron_number;
    steps = unfolding_steps;

    constexpr float std_w = 1e-1f;
    constexpr float std_b = 1e-1f;

    weight = register_module(
        "weight",
        torch::nn::Linear(torch::nn::LinearOptions(input_space, neuron_number).bias(false)));

    recurrent_weight = register_module(
        "recurrent_weight",
        torch::nn::Linear(torch::nn::LinearOptions(neuron_number, neuron_number).bias(false)));

    bias = register_parameter("bias", torch::randn({1, neuron_number}));

    a = register_parameter("a", torch::ones({1, neuron_number}));
    tau = register_parameter("tau", torch::ones({1, neuron_number}));

    torch::nn::init::normal_(weight->weight, 0, std_w / static_cast<float>(unfolding_steps));
    torch::nn::init::normal_(recurrent_weight->weight, 0, std_w / static_cast<float>(unfolding_steps));
    torch::nn::init::normal_(bias, 0, std_b);

    reset_x_t();
}

void LiquidCellModule::reset_x_t() {
    x_t = torch::tanh(
        torch::randn(
            {1, neuron_number}, torch::TensorOptions().device(recurrent_weight->weight.device())));
}

torch::Tensor
LiquidCellModule::compute_step(const torch::Tensor &x_t_curr, const torch::Tensor &i_t) {
    return torch::tanh(weight->forward(i_t) + recurrent_weight->forward(x_t_curr) + bias);
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

// shared network

ActorCriticLiquidNetwork::ActorCriticLiquidNetwork(
    const std::vector<int64_t> &state_space, std::vector<int64_t> action_space, int hidden_size,
    int unfolding_steps) {

    const auto input_space = state_space[0];

    liquid_network = register_module(
        "liquid_network",
        std::make_shared<LiquidCellModule>(input_space, hidden_size, unfolding_steps));

    mu = register_module(
        "mu",
        torch::nn::Sequential(torch::nn::Linear(hidden_size, action_space[0]), torch::nn::Tanh()));

    sigma = register_module(
        "sigma", torch::nn::Sequential(
                     torch::nn::Linear(hidden_size, action_space[0]), torch::nn::Softplus()));

    critic = register_module("critic", torch::nn::Linear(hidden_size, 1));

    mu->apply(init_weights);
    sigma->apply(init_weights);
    critic->apply(init_weights);
}

a2c_response ActorCriticLiquidNetwork::forward(const torch::Tensor &state) {
    const auto x_t = liquid_network->forward(state.unsqueeze(0));

    return {
        mu->forward(x_t).squeeze(0), sigma->forward(x_t).squeeze(0),
        critic->forward(x_t).squeeze(0)};
}

void ActorCriticLiquidNetwork::reset_liquid() const { liquid_network->reset_x_t(); }

// separated networks - actor

ActorLiquidNetwork::ActorLiquidNetwork(
    const std::vector<int64_t> &state_space, std::vector<int64_t> action_space, int hidden_size,
    int unfolding_steps) {

    const auto input_space = state_space[0];

    liquid_network = register_module(
        "liquid_network",
        std::make_shared<LiquidCellModule>(input_space, hidden_size, unfolding_steps));

    mu = register_module(
        "mu",
        torch::nn::Sequential(torch::nn::Linear(hidden_size, action_space[0]), torch::nn::Tanh()));

    sigma = register_module(
        "sigma", torch::nn::Sequential(
                     torch::nn::Linear(hidden_size, action_space[0]), torch::nn::Softplus()));

    mu->apply(init_weights);
    sigma->apply(init_weights);
}

void ActorLiquidNetwork::reset_liquid() const { liquid_network->reset_x_t(); }

actor_response ActorLiquidNetwork::forward(const torch::Tensor &state) {
    const auto x_t = liquid_network->forward(state.unsqueeze(0));

    return {mu->forward(x_t).squeeze(0), sigma->forward(x_t).squeeze(0)};
}

// separated networks - critic

CriticLiquidNetwork::CriticLiquidNetwork(
    const std::vector<int64_t> &state_space, int hidden_size, int unfolding_steps) {

    const auto input_space = state_space[0];

    liquid_network = register_module(
        "liquid_network",
        std::make_shared<LiquidCellModule>(input_space, hidden_size, unfolding_steps));

    critic = register_module("critic", torch::nn::Linear(hidden_size, 1));

    critic->apply(init_weights);
}

void CriticLiquidNetwork::reset_liquid() const { liquid_network->reset_x_t(); }

critic_response CriticLiquidNetwork::forward(const torch::Tensor &state) {
    const auto x_t = liquid_network->forward(state.unsqueeze(0));

    return {critic->forward(x_t).squeeze(0)};
}

/*
 * Agent
 */

ActorCriticLiquid::ActorCriticLiquid(
    const int seed, const std::vector<int64_t> &state_space,
    const std::vector<int64_t> &action_space, int hidden_size, const int batch_size, float lr,
    const float gamma, int unfolding_steps)
    : ActorCritic(seed, state_space, action_space, hidden_size, batch_size, lr, gamma) {

    actor = std::make_shared<ActorLiquidNetwork>(
        state_space, action_space, hidden_size, unfolding_steps);
    actor_optimizer = std::make_shared<torch::optim::Adam>(actor->parameters(), lr);

    critic = std::make_shared<CriticLiquidNetwork>(state_space, hidden_size, unfolding_steps);
    critic_optimizer = std::make_shared<torch::optim::Adam>(critic->parameters(), lr);
}

void ActorCriticLiquid::done(const float reward) {
    ActorCritic::done(reward);

    std::dynamic_pointer_cast<ActorLiquidNetwork>(actor)->reset_liquid();
    std::dynamic_pointer_cast<CriticLiquidNetwork>(critic)->reset_liquid();
}