//
// Created by samuel on 19/12/22.
//

#ifndef EVO_MOTION_ACTOR_CRITIC_H
#define EVO_MOTION_ACTOR_CRITIC_H

#include <memory>
#include <random>
#include <vector>

#include <torch/nn.h>
#include <torch/torch.h>

#include "./agent.h"

struct a2c_response {
    torch::Tensor mu;
    torch::Tensor sigma;
    torch::Tensor critic_value;
};

struct abstract_a2c_networks : torch::nn::Module {
    virtual a2c_response forward(const torch::Tensor &state) = 0;
};

struct a2c_networks : abstract_a2c_networks {
    a2c_networks(
        std::vector<int64_t> state_space, std::vector<int64_t> action_space, int hidden_size);

    a2c_response forward(const torch::Tensor &state) override;

    torch::nn::Sequential head{nullptr};

    torch::nn::Sequential mu{nullptr};
    torch::nn::Sequential sigma{nullptr};

    torch::nn::Sequential critic{nullptr};
};

class ActorCritic : public Agent {
protected:
    std::shared_ptr<abstract_a2c_networks> networks;
    std::shared_ptr<torch::optim::Adam> optimizer;

private:
    torch::DeviceType curr_device;

    float gamma;

    std::vector<a2c_response> results_buffer;
    std::vector<float> rewards_buffer;
    std::vector<torch::Tensor> actions_buffer;

    float episode_actor_loss;
    float episode_critic_loss;

    void train();

public:
    ActorCritic(
        int seed, const std::vector<int64_t> &state_space, const std::vector<int64_t> &action_space,
        int hidden_size, float lr);

    torch::Tensor act(torch::Tensor state, float reward) override;

    void done(float reward) override;

    void save(const std::string &output_folder_path) override;

    void load(const std::string &input_folder_path) override;

    std::map<std::string, float> get_metrics() override;

    void to(torch::DeviceType device) override;

    void set_eval(bool eval) override;
};

#endif//EVO_MOTION_ACTOR_CRITIC_H
