//
// Created by samuel on 29/03/24.
//

#ifndef EVO_MOTION_ACTOR_CRITIC_LIQUID_H
#define EVO_MOTION_ACTOR_CRITIC_LIQUID_H

#include "./actor_critic.h"

struct a2c_liquid_networks : abstract_a2c_networks {
    a2c_liquid_networks(
        std::vector<int64_t> state_space, std::vector<int64_t> action_space, int hidden_size,
        int unfolding_steps);

    void reset_x_t();

    a2c_response forward(const torch::Tensor &state) override;

    torch::Tensor compute_step(const torch::Tensor &x_t, const torch::Tensor &i_t);

    int steps;
    int hidden_size;

    torch::nn::Linear weight{nullptr};
    torch::nn::Linear recurrent_weight{nullptr};
    torch::Tensor bias;
    torch::Tensor a;
    torch::Tensor tau;

    torch::nn::Sequential mu{nullptr};
    torch::nn::Sequential sigma{nullptr};

    torch::nn::Linear critic{nullptr};

    torch::Tensor x_t;
};

class ActorCriticLiquid : public ActorCritic {
public:
    ActorCriticLiquid(
        int seed, const std::vector<int64_t> &state_space, const std::vector<int64_t> &action_space,
        int hidden_size, float lr);

    void done(step step) override;
};

#endif//EVO_MOTION_ACTOR_CRITIC_LIQUID_H
