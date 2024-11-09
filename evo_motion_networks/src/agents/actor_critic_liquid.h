//
// Created by samuel on 29/03/24.
//

#ifndef EVO_MOTION_ACTOR_CRITIC_LIQUID_H
#define EVO_MOTION_ACTOR_CRITIC_LIQUID_H

#include "./actor_critic.h"
#include "./soft_actor_critic.h"

class LiquidCellModule final : public torch::nn::Module {
public:
    LiquidCellModule(const int &input_space, int neuron_number, int unfolding_steps);

    void reset_x_t();

    torch::Tensor compute_step(const torch::Tensor &x_t_curr, const torch::Tensor &i_t);

    torch::Tensor forward(const torch::Tensor &state);

    void to(torch::Device device, bool non_blocking) override;

private:
    int steps;
    int neuron_number;

    torch::nn::Linear weight{nullptr};
    torch::nn::Linear recurrent_weight{nullptr};
    torch::Tensor bias;

    torch::Tensor a;
    torch::Tensor tau;

    torch::Tensor x_t;
};

// shared layers

class ActorCriticLiquidNetwork final : public AbstractActorCritic {
public:
    ActorCriticLiquidNetwork(
        const std::vector<int64_t> &state_space, std::vector<int64_t> action_space, int hidden_size,
        int unfolding_steps);

    a2c_response forward(const torch::Tensor &state) override;

    void reset_liquid() const;

private:
    std::shared_ptr<LiquidCellModule> liquid_network;

    torch::nn::Sequential mu{nullptr};
    torch::nn::Sequential sigma{nullptr};

    torch::nn::Linear critic{nullptr};
};

// separated networks

class ActorLiquidNetwork : public AbstractActor {
public:
    ActorLiquidNetwork(
        const std::vector<int64_t> &state_space, std::vector<int64_t> action_space, int hidden_size,
        int unfolding_steps);

    void reset_liquid() const;

    actor_response forward(const torch::Tensor &state) override;

private:
    std::shared_ptr<LiquidCellModule> liquid_network;

    torch::nn::Sequential mu{nullptr};
    torch::nn::Sequential sigma{nullptr};
};

class CriticLiquidNetwork : public AbstractCritic {
public:
    CriticLiquidNetwork(
        const std::vector<int64_t> &state_space, int hidden_size, int unfolding_steps);

    void reset_liquid() const;

    critic_response forward(const torch::Tensor &state) override;

private:
    std::shared_ptr<LiquidCellModule> liquid_network;
    torch::nn::Linear critic{nullptr};
};

class QNetworkLiquid : public AbstractQNetwork {
public:
    QNetworkLiquid(
        const std::vector<int64_t> &state_space, std::vector<int64_t> action_space, int hidden_size,
        int unfolding_steps);

    critic_response forward(const torch::Tensor &state, const torch::Tensor &action) override;

    void reset_liquid() const;

private:
    std::shared_ptr<LiquidCellModule> liquid_network{nullptr};
    torch::nn::Linear q_network{nullptr};
};

// Agent

class ActorCriticLiquid final : public ActorCritic {
public:
    ActorCriticLiquid(
        int seed, const std::vector<int64_t> &state_space, const std::vector<int64_t> &action_space,
        int hidden_size, int batch_size, float lr, float gamma, float entropy_start_factor,
        float entropy_end_factor, long entropy_steps, int unfolding_steps);

    void done(torch::Tensor state, float reward) override;
};

class SoftActorCriticLiquid : public SoftActorCritic {
public:
    SoftActorCriticLiquid(
        int seed, const std::vector<int64_t> &state_space, const std::vector<int64_t> &action_space,
        int hidden_size, int batch_size, float lr, float gamma, float tau, int unfolding_steps);

    void done(torch::Tensor state, float reward) override;
};

#endif//EVO_MOTION_ACTOR_CRITIC_LIQUID_H