//
// Created by samuel on 11/11/24.
//

#include <evo_motion_networks/networks/entropy.h>

EntropyParameter::EntropyParameter(const float initial_alpha) {
    log_alpha_t = register_parameter(
        "log_alpha", torch::tensor(
                        {std::log(initial_alpha)},
                         at::TensorOptions().requires_grad(true)));
}

torch::Tensor EntropyParameter::log_alpha() { return log_alpha_t; }

torch::Tensor EntropyParameter::alpha() { return log_alpha().exp(); }