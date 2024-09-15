//
// Created by samuel on 14/04/24.
//

#include "./debug_agents.h"

// Debug agent
DebugAgent::DebugAgent(
    int seed, const std::vector<int64_t> &state_space, const std::vector<int64_t> &action_space,
    int hidden_size, float lr)
    : action_space(static_cast<int>(action_space[0])), curr_device(torch::kCPU) {}

void DebugAgent::done(float reward) {}

void DebugAgent::save(const std::string &output_folder_path) {}

void DebugAgent::load(const std::string &input_folder_path) {}

std::map<std::string, float> DebugAgent::get_metrics() { return {}; }

void DebugAgent::to(const torch::DeviceType device) { curr_device = device; }

void DebugAgent::set_eval(bool eval) {}

int DebugAgent::count_parameters() { return 0; }

// Random Agent
RandomAgent::RandomAgent(
    const int seed, const std::vector<int64_t> &state_space,
    const std::vector<int64_t> &action_space, const int hidden_size, const float lr)
    : DebugAgent(seed, state_space, action_space, hidden_size, lr) {}

torch::Tensor RandomAgent::act(torch::Tensor state, float reward) {
    return 2.f * torch::rand({action_space}, torch::TensorOptions(curr_device)) - 1.f;
}

// Constant Agent
ConstantAgent::ConstantAgent(
    const int seed, const std::vector<int64_t> &state_space,
    const std::vector<int64_t> &action_space, const int hidden_size, const float lr)
    : DebugAgent(seed, state_space, action_space, hidden_size, lr) {}

torch::Tensor ConstantAgent::act(torch::Tensor state, float reward) {
    return torch::zeros({action_space}, torch::TensorOptions(curr_device));
}