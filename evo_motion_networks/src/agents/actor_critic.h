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

#include <evo_motion_networks/agent.h>

// responses

struct actor_response {
    torch::Tensor mu;
    torch::Tensor sigma;
};

struct critic_response {
    torch::Tensor value;
};

struct a2c_response {
    torch::Tensor mu;
    torch::Tensor sigma;
    torch::Tensor value;
};

// abstract modules

class AbstractActorCritic : public torch::nn::Module {
public:
    virtual a2c_response forward(const torch::Tensor &state) = 0;
};

class AbstractActor : public torch::nn::Module {
public:
    virtual actor_response forward(const torch::Tensor &state) = 0;
};

class AbstractCritic : public torch::nn::Module {
public:
    virtual critic_response forward(const torch::Tensor &state) = 0;
};

// module

class ActorCriticModule : public AbstractActorCritic {
public:
    ActorCriticModule(
        std::vector<int64_t> state_space, std::vector<int64_t> action_space, int hidden_size);

    a2c_response forward(const torch::Tensor &state) override;

private:
    torch::nn::Sequential head{nullptr};

    torch::nn::Sequential mu{nullptr};
    torch::nn::Sequential sigma{nullptr};

    torch::nn::Sequential critic{nullptr};
};

class ActorModule : public AbstractActor {
public:
    ActorModule(
        std::vector<int64_t> state_space, std::vector<int64_t> action_space, int hidden_size);

    actor_response forward(const torch::Tensor &state) override;

private:
    torch::nn::Sequential head{nullptr};

    torch::nn::Sequential mu{nullptr};
    torch::nn::Sequential sigma{nullptr};
};

class CriticModule : public AbstractCritic {
public:
    CriticModule(
        std::vector<int64_t> state_space, int hidden_size);

    critic_response forward(const torch::Tensor &state) override;

private:
    torch::nn::Sequential critic{nullptr};
};

// Agent

class ActorCritic : public Agent {
protected:
    //std::shared_ptr<AbstractActorCritic> actor_critic;
    //std::shared_ptr<torch::optim::Adam> optimizer;
    std::shared_ptr<AbstractActor> actor;
    std::shared_ptr<torch::optim::Optimizer> actor_optimizer;

    std::shared_ptr<AbstractCritic> critic;
    std::shared_ptr<torch::optim::Optimizer> critic_optimizer;

private:
    float gamma;
    float first_entropy_factor;
    float wanted_entropy_factor;
    long entropy_factor_steps;

    torch::DeviceType curr_device;

    std::vector<a2c_response> results_buffer;
    std::vector<float> rewards_buffer;
    std::vector<torch::Tensor> actions_buffer;

    float episode_policy_loss;
    float episode_policy_entropy;
    float episode_critic_loss;

    long curr_step;

    void train();

    float get_exponential_entropy_factor() const;

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

    int count_parameters() override;
};

#endif//EVO_MOTION_ACTOR_CRITIC_H