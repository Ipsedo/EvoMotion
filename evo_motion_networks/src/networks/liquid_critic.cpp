//
// Created by samuel on 11/11/24.
//

#include <evo_motion_networks/init.h>
#include <evo_motion_networks/networks/critic.h>

CriticLiquidModule::CriticLiquidModule(
    const std::vector<int64_t> &state_space, int hidden_size, int unfolding_steps) {

    const auto input_space = state_space[0];

    liquid_network = register_module(
        "liquid_network",
        std::make_shared<LiquidCellModule>(input_space, hidden_size, unfolding_steps));

    critic = register_module("critic", torch::nn::Linear(hidden_size, 1));

    critic->apply(init_weights);
}

void CriticLiquidModule::reset_liquid() const { liquid_network->reset_x_t(); }

torch::Tensor CriticLiquidModule::get_x() const { return liquid_network->get_x_t(); }

critic_response CriticLiquidModule::forward(const torch::Tensor &state) {
    const auto x_t = liquid_network->get_x_t().squeeze(0);

    const auto [out, new_x_t] = forward(x_t, state);

    liquid_network->set_x_t(new_x_t.unsqueeze(0));

    return {out};
}

liquid_critic_response
CriticLiquidModule::forward(const torch::Tensor &x_t, const torch::Tensor &state) {
    bool only_one = false;

    auto curr_x_t = x_t;
    auto liquid_input = state;

    if (liquid_input.sizes().size() == 1) {
        only_one = true;
        curr_x_t = curr_x_t.unsqueeze(0);
        liquid_input = liquid_input.unsqueeze(0);
    }

    auto next_x_t = liquid_network->forward(curr_x_t, liquid_input);
    auto q_value = critic->forward(next_x_t);

    if (only_one) {
        next_x_t = next_x_t.squeeze(0);
        q_value = q_value.squeeze(0);
    }

    return {q_value, next_x_t};
}