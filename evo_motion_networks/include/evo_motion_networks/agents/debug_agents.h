//
// Created by samuel on 14/04/24.
//

#ifndef EVO_MOTION_DEBUG_AGENTS_H
#define EVO_MOTION_DEBUG_AGENTS_H

#include <evo_motion_networks/agent.h>

class DebugAgent : public Agent {
public:
    explicit DebugAgent(const std::vector<int64_t> &action_space);

    void done(torch::Tensor state, float reward) override;

    void save(const std::string &output_folder_path) override;

    void load(const std::string &input_folder_path) override;

    std::vector<LossMeter> get_metrics() override;

    void to(torch::DeviceType device) override;

    void set_eval(bool eval) override;

    int count_parameters() override;

protected:
    int action_space;

    torch::DeviceType curr_device;
};

class RandomAgent final : public DebugAgent {
public:
    explicit RandomAgent(const std::vector<int64_t> &action_space);

    torch::Tensor act(torch::Tensor state, float reward) override;
};

class ConstantAgent final : public DebugAgent {
public:
    explicit ConstantAgent(const std::vector<int64_t> &action_space, const float &action_value);

    torch::Tensor act(torch::Tensor state, float reward) override;

private:
    float action_value;
};

#endif//EVO_MOTION_DEBUG_AGENTS_H