//
// Created by samuel on 29/03/24.
//

#include "./actor_critic_liquid.h"

#include "../init.h"

a2c_liquid_networks::a2c_liquid_networks(
    std::vector<int64_t> state_space, std::vector<int64_t> action_space, int neuron_number,
    int unfolding_steps) {
    hidden_size = neuron_number;
    steps = unfolding_steps;

    weight = register_module(
        "weight",
        torch::nn::Linear(torch::nn::LinearOptions(state_space[0], hidden_size).bias(false)));

    recurrent_weight = register_module(
        "recurrent_weight",
        torch::nn::Linear(torch::nn::LinearOptions(hidden_size, hidden_size).bias(false)));

    bias = register_parameter("bias", torch::randn({1, hidden_size}));

    a = register_parameter("a", torch::ones({1, hidden_size}));
    tau = register_parameter("tau", torch::ones({1, hidden_size}));

    mu = register_module(
        "mu", torch::nn::Sequential(
            torch::nn::Linear(hidden_size, action_space[0]), torch::nn::Tanh()));

    sigma = register_module(
        "sigma", torch::nn::Sequential(
            torch::nn::Linear(hidden_size, action_space[0]), torch::nn::Softplus()));

    critic = register_module(
        "critic", torch::nn::Linear(hidden_size, 1));

    x_t = torch::mish(torch::randn({1, hidden_size}));

    this->apply(init_weights);
}

torch::Tensor
a2c_liquid_networks::compute_step(const torch::Tensor &x_t_curr, const torch::Tensor &i_t) {
    return torch::mish(weight->forward(i_t) + recurrent_weight->forward(x_t_curr) + bias);
}

a2c_response a2c_liquid_networks::forward(const torch::Tensor &state) {
    auto batched_state = state.unsqueeze(0);

    float delta_t = 1.f / float(steps);

    for (int i = 0; i < steps; i++)
        x_t = (x_t + delta_t * compute_step(x_t, batched_state) * a)
              / (1.f + delta_t * (1.f / tau + compute_step(x_t, batched_state)));

    return {
        mu->forward(x_t).squeeze(0), sigma->forward(x_t).squeeze(0),
        critic->forward(x_t).squeeze(0)};
}

void a2c_liquid_networks::reset_x_t() {
    x_t = torch::mish(
        torch::randn(
            {1, hidden_size}, torch::TensorOptions().device(recurrent_weight->weight.device())));
}

void a2c_liquid_networks::to(torch::Device device, bool non_blocking) {
    Module::to(device, non_blocking);
    x_t = x_t.to(device, non_blocking);
}

ActorCriticLiquid::ActorCriticLiquid(
    int seed, const std::vector<int64_t> &state_space, const std::vector<int64_t> &action_space,
    int hidden_size, float lr)
    : ActorCritic(seed, state_space, action_space, hidden_size, lr) {
    networks = std::make_shared<a2c_liquid_networks>(state_space, action_space, hidden_size, 6);
    optimizer = std::make_shared<torch::optim::Adam>(networks->parameters(), lr);

    actor_loss_factor = 1.f;
    critic_loss_factor = 1.f;
}

void ActorCriticLiquid::done(float reward) {
    ActorCritic::done(reward);

    std::dynamic_pointer_cast<a2c_liquid_networks>(networks)->reset_x_t();
}