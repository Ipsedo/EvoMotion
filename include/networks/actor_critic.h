//
// Created by samuel on 19/12/22.
//

#ifndef EVO_MOTION_ACTOR_CRITIC_H
#define EVO_MOTION_ACTOR_CRITIC_H

#include <memory>
#include <vector>

#include <torch/torch.h>
#include <torch/nn.h>

#include "./networks/agent.h"

struct a2c_response {
    torch::Tensor mu;
    torch::Tensor sigma;
    torch::Tensor critic_value;
};

struct a2c_networks : torch::nn::Module {
    a2c_networks(std::vector<int64_t> state_space, std::vector<int64_t> action_space, int hidden_size);

    a2c_response forward(const torch::Tensor& state);

    torch::nn::Linear head{nullptr};

    torch::nn::Linear critic{nullptr};

    torch::nn::Linear mu{nullptr};
    torch::nn::Linear sigma{nullptr};
};

class ActorCritic : public Agent {
private:
    float gamma;

    std::shared_ptr<a2c_networks> networks;
    torch::optim::Adam optimizer;

    std::vector<a2c_response> results_buffer;
    std::vector<float> rewards_buffer;

    void train();
public:
    ActorCritic(
            const std::vector<int64_t>& state_space,
            const std::vector<int64_t>& action_space,
            int hidden_size,
            float lr
            );
    torch::Tensor act(step step) override;

    void done(step step) override;

};


#endif //EVO_MOTION_ACTOR_CRITIC_H
