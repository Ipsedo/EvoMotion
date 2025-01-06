//
// Created by samuel on 11/11/24.
//

#include <evo_motion_networks/networks/entropy.h>

EntropyParameter::EntropyParameter(
    const float initial_alpha, const int nb_parameters, const float min_alpha,
    const float max_alpha)
    : log_alpha_t(register_parameter(
          "log_alpha", torch::tensor(
                           std::vector<float>(nb_parameters, std::log(initial_alpha)),
                           at::TensorOptions().requires_grad(true)))),
      min_log(std::log(min_alpha)), max_log(std::log(max_alpha)) {}

torch::Tensor EntropyParameter::log_alpha() { return torch::clamp(log_alpha_t, min_log, max_log); }

torch::Tensor EntropyParameter::alpha() { return log_alpha().exp(); }