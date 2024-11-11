//
// Created by samuel on 11/11/24.
//

#include <evo_motion_networks/networks/liquid.h>

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
    torch::nn::init::normal_(
        recurrent_weight->weight, 0, std_w / static_cast<float>(unfolding_steps));
    torch::nn::init::normal_(bias, 0, std_b);

    reset_x_t();
}

void LiquidCellModule::reset_x_t() { x_t = gen_first_x_t(1); }

torch::Tensor LiquidCellModule::gen_first_x_t(int batch_size) {
    return torch::silu(torch::zeros(
        {batch_size, neuron_number},
        torch::TensorOptions().device(recurrent_weight->weight.device())));
}

torch::Tensor
LiquidCellModule::compute_step(const torch::Tensor &x_t_curr, const torch::Tensor &i_t) {
    return torch::silu(weight->forward(i_t) + recurrent_weight->forward(x_t_curr) + bias);
}

torch::Tensor LiquidCellModule::forward(const torch::Tensor &state) {
    x_t = forward(x_t, state);
    return x_t;
}

torch::Tensor LiquidCellModule::forward(const torch::Tensor &last_x_t, const torch::Tensor &state) {
    const float delta_t = 1.f / static_cast<float>(steps);

    auto curr_x_t = last_x_t;
    for (int i = 0; i < steps; i++)
        curr_x_t = (curr_x_t + delta_t * compute_step(curr_x_t, state) * a)
                   / (1.f + delta_t * (1.f / tau + compute_step(curr_x_t, state)));

    return curr_x_t;
}

void LiquidCellModule::to(torch::Device device, const bool non_blocking) {
    Module::to(device, non_blocking);
    x_t = x_t.to(device, non_blocking);
}

torch::Tensor LiquidCellModule::get_x_t() { return x_t; }

void LiquidCellModule::set_x_t(const torch::Tensor &new_x_t) { x_t = new_x_t; }
