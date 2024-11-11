//
// Created by samuel on 11/11/24.
//

#include <evo_motion_networks/networks/entropy.h>

EntropyParameter::EntropyParameter() {
    log_alpha_t =
        register_parameter("log_alpha", torch::zeros({1}, at::TensorOptions().requires_grad(true)));
}

torch::Tensor EntropyParameter::log_alpha() { return log_alpha_t; }

torch::Tensor EntropyParameter::alpha() { return log_alpha().exp().detach(); }