//
// Created by samuel on 14/04/24.
//

#ifndef EVO_MOTION_DEBUG_AGENTS_H
#define EVO_MOTION_DEBUG_AGENTS_H

#include <evo_motion_networks/agent.h>

class DebugAgent : public Agent {
public:
    DebugAgent(
        int seed, const std::vector<int64_t> &state_space, const std::vector<int64_t> &action_space,
        int hidden_size, float lr);

    void done(float reward) override;

    void save(const std::string &output_folder_path) override;

    void load(const std::string &input_folder_path) override;

    std::map<std::string, float> get_metrics() override;

    void to(torch::DeviceType device) override;

    void set_eval(bool eval) override;

    int count_parameters() override;

protected:
    int action_space;

    torch::DeviceType curr_device;
};

class RandomAgent final : public DebugAgent {
public:
    RandomAgent(
        int seed, const std::vector<int64_t> &state_space, const std::vector<int64_t> &action_space,
        int hidden_size, int batch_size, float lr);

    torch::Tensor act(torch::Tensor state, float reward) override;
};

class ConstantAgent final : public DebugAgent {
public:
    ConstantAgent(
        int seed, const std::vector<int64_t> &state_space, const std::vector<int64_t> &action_space,
        int hidden_size, int batch_size, float lr);

    torch::Tensor act(torch::Tensor state, float reward) override;
};

#endif//EVO_MOTION_DEBUG_AGENTS_H