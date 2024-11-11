//
// Created by samuel on 11/11/24.
//

#ifndef EVO_MOTION_CRITIC_H
#define EVO_MOTION_CRITIC_H

#include <torch/torch.h>

#include <evo_motion_networks/networks/liquid.h>

// response

struct critic_response {
    torch::Tensor value;
};

struct liquid_critic_response {
    torch::Tensor q_value;
    torch::Tensor new_x_t;
};

// abstract module

class AbstractCritic : public torch::nn::Module {
public:
    virtual critic_response forward(const torch::Tensor &state) = 0;
};

// Linear

class CriticModule : public AbstractCritic {
public:
    CriticModule(std::vector<int64_t> state_space, int hidden_size);

    critic_response forward(const torch::Tensor &state) override;

private:
    torch::nn::Sequential critic{nullptr};
};

// Liquid

class CriticLiquidModule : public AbstractCritic {
public:
    CriticLiquidModule(
        const std::vector<int64_t> &state_space, int hidden_size, int unfolding_steps);

    void reset_liquid() const;
    torch::Tensor get_x();

    critic_response forward(const torch::Tensor &state) override;
    liquid_critic_response forward(const torch::Tensor &x_t, const torch::Tensor &state);

private:
    std::shared_ptr<LiquidCellModule> liquid_network;
    torch::nn::Linear critic{nullptr};
};

#endif//EVO_MOTION_CRITIC_H
