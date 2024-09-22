//
// Created by samuel on 14/04/24.
//

#include "./debug_agents.h"

// Debug agent
DebugAgent::DebugAgent(const std::vector<int64_t> &action_space)
    : action_space(static_cast<int>(action_space[0])), curr_device(torch::kCPU) {}

void DebugAgent::done(float reward) {}

void DebugAgent::save(const std::string &output_folder_path) {}

void DebugAgent::load(const std::string &input_folder_path) {}

std::map<std::string, float> DebugAgent::get_metrics() { return {}; }

void DebugAgent::to(const torch::DeviceType device) { curr_device = device; }

void DebugAgent::set_eval(bool eval) {}

int DebugAgent::count_parameters() { return 0; }

// Random Agent
RandomAgent::RandomAgent(const std::vector<int64_t> &action_space) : DebugAgent(action_space) {}

torch::Tensor RandomAgent::act(torch::Tensor state, float reward) {
    return 2.f * torch::rand({action_space}, torch::TensorOptions(curr_device)) - 1.f;
}

// Constant Agent
ConstantAgent::ConstantAgent(const std::vector<int64_t> &action_space) : DebugAgent(action_space) {}

torch::Tensor ConstantAgent::act(torch::Tensor state, float reward) {
    return torch::zeros({action_space}, torch::TensorOptions(curr_device));
}