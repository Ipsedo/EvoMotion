//
// Created by samuel on 11/11/24.
//

#include <evo_motion_networks/init.h>
#include <evo_motion_networks/networks/q_net.h>

QNetworkLiquidModule::QNetworkLiquidModule(
    const std::vector<int64_t> &state_space, std::vector<int64_t> action_space, int hidden_size,
    int unfolding_steps) {
    const auto input_space = state_space[0] + action_space[0];

    liquid_network = register_module(
        "liquid_network",
        std::make_shared<LiquidCellModule>(input_space, hidden_size, unfolding_steps));

    q_network = register_module("critic", torch::nn::Linear(hidden_size, 1));

    q_network->apply(init_weights);
}

critic_response
QNetworkLiquidModule::forward(const torch::Tensor &state, const torch::Tensor &action) {
    const auto x_t = liquid_network->get_x_t().squeeze(0);

    const auto [out, new_x_t] = forward(x_t, state, action);

    liquid_network->set_x_t(new_x_t.unsqueeze(0));

    return {out};
}

void QNetworkLiquidModule::reset_liquid() const { liquid_network->reset_x_t(); }

torch::Tensor QNetworkLiquidModule::get_x() { return liquid_network->get_x_t(); }

liquid_critic_response QNetworkLiquidModule::forward(
    const torch::Tensor &x_t, const torch::Tensor &state, const torch::Tensor &action) {
    bool only_one = false;

    auto curr_x_t = x_t;
    auto liquid_input = torch::cat({state, action}, -1);

    if (liquid_input.sizes().size() == 1) {
        only_one = true;
        curr_x_t = curr_x_t.unsqueeze(0);
        liquid_input = liquid_input.unsqueeze(0);
    }

    auto next_x_t = liquid_network->forward(curr_x_t, liquid_input);
    auto q_value = q_network->forward(next_x_t);

    if (only_one) {
        next_x_t = next_x_t.squeeze(0);
        q_value = q_value.squeeze(0);
    }

    return {q_value, next_x_t};
}