//
// Created by samuel on 11/11/24.
//

#include <evo_motion_networks/init.h>
#include <evo_motion_networks/networks/actor.h>
#include <evo_motion_networks/networks/misc.h>

ActorLiquidModule::ActorLiquidModule(
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
                     torch::nn::Linear(hidden_size, action_space[0]),
                     torch::nn::Softplus(torch::nn::SoftplusOptions().beta(1.f).threshold(20.f)),
                     ClampModule(1e-8, 1e2)));

    mu->apply(init_weights);
    sigma->apply(init_weights);
}

void ActorLiquidModule::reset_liquid() const { liquid_network->reset_x_t(); }

torch::Tensor ActorLiquidModule::get_x() const { return liquid_network->get_x_t(); }

actor_response ActorLiquidModule::forward(const torch::Tensor &state) {
    const auto x_t = liquid_network->get_x_t().squeeze(0);

    const auto [out_mu, out_sigma, new_x_t] = forward(x_t, state);

    liquid_network->set_x_t(new_x_t.unsqueeze(0));

    return {out_mu, out_sigma};
}

liquid_actor_response
ActorLiquidModule::forward(const torch::Tensor &x_t, const torch::Tensor &state) {
    bool only_one = false;

    auto curr_x_t = x_t;
    auto curr_state = state;
    if (curr_state.sizes().size() == 1) {
        only_one = true;
        curr_x_t = curr_x_t.unsqueeze(0);
        curr_state = curr_state.unsqueeze(0);
    }

    auto new_x_t = liquid_network->forward(curr_x_t, curr_state);
    auto out_mu = mu->forward(new_x_t);
    auto out_sigma = sigma->forward(new_x_t);

    if (only_one) {
        new_x_t = new_x_t.squeeze(0);
        out_mu = out_mu.squeeze(0);
        out_sigma = out_sigma.squeeze(0);
    }

    return {out_mu, out_sigma, new_x_t};
}