//
// Created by samuel on 29/03/24.
//

#ifndef EVO_MOTION_ACTOR_CRITIC_LIQUID_H
#define EVO_MOTION_ACTOR_CRITIC_LIQUID_H

#include "./actor_critic.h"

class LiquidCellModule final : public torch::nn::Module {
public:
    LiquidCellModule(const std::vector<int64_t> &state_space, int neuron_number, int unfolding_steps);
    void reset_x_t();
    torch::Tensor compute_step(const torch::Tensor &x_t_curr, const torch::Tensor &i_t);
    torch::Tensor forward(const torch::Tensor &state);
    void to(torch::Device device, bool non_blocking) override;

private:
    int steps;
    int neuron_number;

    torch::nn::Sequential weight{nullptr};
    torch::nn::Linear recurrent_weight{nullptr};
    torch::Tensor bias;
    torch::Tensor a;
    torch::Tensor tau;

    torch::Tensor x_t;
};

class ActorCriticLiquidModule final : public AbstractActorCriticModule {
public:
    ActorCriticLiquidModule(
        const std::vector<int64_t> &state_space, std::vector<int64_t> action_space, int hidden_size,
        int unfolding_steps);

    a2c_response forward(const torch::Tensor &state) override;

    void reset_liquid() const;

private:
    std::shared_ptr<LiquidCellModule> liquid_network;

    torch::nn::Sequential mu{nullptr};
    torch::nn::Sequential sigma{nullptr};

    torch::nn::Sequential critic{nullptr};
};

// Agent

class ActorCriticLiquid final : public ActorCritic {
public:
    ActorCriticLiquid(
        int seed, const std::vector<int64_t> &state_space, const std::vector<int64_t> &action_space,
        int hidden_size, float lr);

    void done(float reward) override;
};

#endif//EVO_MOTION_ACTOR_CRITIC_LIQUID_H
