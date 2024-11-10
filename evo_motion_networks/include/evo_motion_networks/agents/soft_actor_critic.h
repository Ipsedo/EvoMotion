//
// Created by samuel on 08/11/24.
//

#ifndef EVO_MOTION_SOFT_ACTOR_CRITIC_H
#define EVO_MOTION_SOFT_ACTOR_CRITIC_H

#include <torch/torch.h>

#include <evo_motion_networks/agent.h>

#include "./actor_critic.h"

struct soft_episode_buffer {
    std::vector<torch::Tensor> mu_buffer;
    std::vector<torch::Tensor> sigma_buffer;
    std::vector<torch::Tensor> q_value_1_buffer;
    std::vector<torch::Tensor> q_value_2_buffer;
    std::vector<torch::Tensor> target_q_value_1_buffer;
    std::vector<torch::Tensor> target_q_value_2_buffer;
    std::vector<float> rewards_buffer;
    std::vector<torch::Tensor> actions_buffer;
    std::vector<float> done_buffer;
};

class AbstractQNetwork : public torch::nn::Module {
public:
    virtual critic_response forward(const torch::Tensor &state, const torch::Tensor &action) = 0;
};

class QNetworkModule : public AbstractQNetwork {
public:
    QNetworkModule(
        std::vector<int64_t> state_space, std::vector<int64_t> action_space, int hidden_size);

    critic_response forward(const torch::Tensor &state, const torch::Tensor &action) override;

private:
    torch::nn::Sequential q_network{nullptr};
};

class EntropyParameter : public torch::nn::Module {
public:
    EntropyParameter();
    torch::Tensor log_alpha();
    torch::Tensor alpha();
};

class SoftActorCriticAgent : public Agent {
protected:
    std::shared_ptr<AbstractActor> actor;
    std::shared_ptr<AbstractQNetwork> critic_1;
    std::shared_ptr<AbstractQNetwork> critic_2;
    std::shared_ptr<AbstractQNetwork> target_critic_1;
    std::shared_ptr<AbstractQNetwork> target_critic_2;

    std::shared_ptr<torch::optim::Optimizer> actor_optimizer;
    std::shared_ptr<torch::optim::Optimizer> critic_1_optimizer;
    std::shared_ptr<torch::optim::Optimizer> critic_2_optimizer;

private:
    float target_entropy;
    EntropyParameter entropy_parameter;
    torch::optim::Adam entropy_optimizer;

    torch::DeviceType curr_device;

    float gamma;
    float tau;
    int batch_size;
    std::vector<soft_episode_buffer> episodes_buffer;

    int curr_episode_step;
    long curr_train_step;

    LossMeter actor_loss_meter;
    LossMeter critic_1_loss_meter;
    LossMeter critic_2_loss_meter;
    LossMeter entropy_loss_meter;
    LossMeter episode_steps_meter;

    void train(
        const torch::Tensor &batched_actions, const torch::Tensor &batched_q_values_1,
        const torch::Tensor &batched_q_values_2, const torch::Tensor &batched_target_q_values_1,
        const torch::Tensor &batched_target_q_values_2, const torch::Tensor &batched_mus,
        const torch::Tensor &batched_sigmas, const torch::Tensor &batched_rewards,
        const torch::Tensor &batched_done);

public:
    SoftActorCriticAgent(
        int seed, const std::vector<int64_t> &state_space, const std::vector<int64_t> &action_space,
        int hidden_size, int batch_size, float lr, float gamma, float tau);

    torch::Tensor act(torch::Tensor state, float reward) override;

    void done(torch::Tensor state, float reward) override;

    void save(const std::string &output_folder_path) override;

    void load(const std::string &input_folder_path) override;

    std::vector<LossMeter> get_metrics() override;

    void to(torch::DeviceType device) override;

    void set_eval(bool eval) override;

    int count_parameters() override;
};

#endif//EVO_MOTION_SOFT_ACTOR_CRITIC_H
