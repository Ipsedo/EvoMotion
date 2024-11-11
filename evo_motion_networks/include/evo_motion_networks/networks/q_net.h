//
// Created by samuel on 11/11/24.
//

#ifndef EVO_MOTION_Q_NET_H
#define EVO_MOTION_Q_NET_H

#include <torch/torch.h>

#include "./critic.h"

// abstract

class AbstractQNetwork : public torch::nn::Module {
public:
    virtual critic_response forward(const torch::Tensor &state, const torch::Tensor &action) = 0;
};

// Linear

class QNetworkModule : public AbstractQNetwork {
public:
    QNetworkModule(
        std::vector<int64_t> state_space, std::vector<int64_t> action_space, int hidden_size);

    critic_response forward(const torch::Tensor &state, const torch::Tensor &action) override;

private:
    torch::nn::Sequential q_network{nullptr};
};

// Liquid

class QNetworkLiquidModule : public AbstractQNetwork {
public:
    QNetworkLiquidModule(
        const std::vector<int64_t> &state_space, std::vector<int64_t> action_space, int hidden_size,
        int unfolding_steps);

    critic_response forward(const torch::Tensor &state, const torch::Tensor &action) override;
    liquid_critic_response
    forward(const torch::Tensor &x_t, const torch::Tensor &state, const torch::Tensor &action);

    void reset_liquid() const;
    torch::Tensor get_x();

private:
    std::shared_ptr<LiquidCellModule> liquid_network{nullptr};
    torch::nn::Linear q_network{nullptr};
};

#endif//EVO_MOTION_Q_NET_H
