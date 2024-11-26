//
// Created by samuel on 26/11/24.
//

#include <evo_motion_networks/networks/misc.h>

// Exp

ExpModule::ExpModule(const float epsilon) : epsilon(epsilon) {}

torch::Tensor ExpModule::forward(const torch::Tensor &input) { return torch::exp(input) + epsilon; }

// Clamp
ClampModule::ClampModule(const float min_value, const float max_value)
    : min_value(min_value), max_value(max_value) {}

torch::Tensor ClampModule::forward(const torch::Tensor &input) {
    return torch::clamp(input, min_value, max_value);
}
