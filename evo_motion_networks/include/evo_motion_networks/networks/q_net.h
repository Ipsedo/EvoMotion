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
    virtual std::shared_ptr<AbstractQNetwork> clone() = 0;
};

// Linear

class QNetworkModule final : public AbstractQNetwork {
public:
    QNetworkModule(
        const std::vector<int64_t> &state_space, const std::vector<int64_t> &action_space,
        int hidden_size);

    critic_response forward(const torch::Tensor &state, const torch::Tensor &action) override;

    std::shared_ptr<AbstractQNetwork> clone() override;

private:
    std::vector<int64_t> state_space;
    std::vector<int64_t> action_space;
    int hidden_size;

    torch::nn::Sequential q_network{nullptr};
};

// BatchNorm -> CrossQ

class BatchNormQNetworkModule final : public AbstractQNetwork {
public:
    BatchNormQNetworkModule(
        const std::vector<int64_t> &state_space, const std::vector<int64_t> &action_space,
        int hidden_size);

    critic_response forward(const torch::Tensor &state, const torch::Tensor &action) override;

    std::shared_ptr<AbstractQNetwork> clone() override;

private:
    std::vector<int64_t> state_space;
    std::vector<int64_t> action_space;
    int hidden_size;

    torch::nn::Sequential q_network{nullptr};
};

// Liquid

class QNetworkLiquidModule final : public AbstractQNetwork {
public:
    QNetworkLiquidModule(
        const std::vector<int64_t> &state_space, const std::vector<int64_t> &action_space,
        int hidden_size, int unfolding_steps);

    critic_response forward(const torch::Tensor &state, const torch::Tensor &action) override;
    liquid_critic_response
    forward(const torch::Tensor &x_t, const torch::Tensor &state, const torch::Tensor &action);

    void reset_liquid() const;
    torch::Tensor get_x() const;

    std::shared_ptr<AbstractQNetwork> clone() override;

private:
    std::vector<int64_t> state_space;
    std::vector<int64_t> action_space;
    int hidden_size;
    int unfolding_steps;

    std::shared_ptr<LiquidCellModule> liquid_network{nullptr};
    torch::nn::Linear q_network{nullptr};
};

// KAN

class QNetworkKanModule final : public AbstractQNetwork {
public:
    QNetworkKanModule(
        const std::vector<int64_t> &state_space, const std::vector<int64_t> &action_space,
        int hidden_size, int poly_degree, int grid_size);

    critic_response forward(const torch::Tensor &state, const torch::Tensor &action) override;

    std::shared_ptr<AbstractQNetwork> clone() override;

private:
    std::vector<int64_t> state_space;
    std::vector<int64_t> action_space;
    int hidden_size;

    int poly_degree;
    int grid_size;

    torch::nn::Sequential q_network{nullptr};
};

#endif//EVO_MOTION_Q_NET_H
