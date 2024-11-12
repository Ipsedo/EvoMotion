//
// Created by samuel on 08/11/24.
//

#ifndef EVO_MOTION_SOFT_ACTOR_CRITIC_H
#define EVO_MOTION_SOFT_ACTOR_CRITIC_H

#include <random>

#include <torch/torch.h>

#include <evo_motion_networks/agent.h>
#include <evo_motion_networks/networks/actor.h>
#include <evo_motion_networks/networks/entropy.h>
#include <evo_motion_networks/networks/q_net.h>
#include <evo_motion_networks/replay_buffer.h>

class SoftActorCriticAgent : public Agent {
private:
    std::shared_ptr<AbstractActor> actor;
    std::shared_ptr<AbstractQNetwork> critic_1;
    std::shared_ptr<AbstractQNetwork> critic_2;
    std::shared_ptr<AbstractQNetwork> target_critic_1;
    std::shared_ptr<AbstractQNetwork> target_critic_2;

    std::shared_ptr<torch::optim::Optimizer> actor_optimizer;
    std::shared_ptr<torch::optim::Optimizer> critic_1_optimizer;
    std::shared_ptr<torch::optim::Optimizer> critic_2_optimizer;

    float target_entropy;
    EntropyParameter entropy_parameter;
    torch::optim::Adam entropy_optimizer;

    torch::DeviceType curr_device;

    float gamma;
    float tau;
    int batch_size;
    ReplayBuffer replay_buffer;

    int curr_episode_step;
    long curr_train_step;
    long global_curr_step;

    LossMeter actor_loss_meter;
    LossMeter critic_1_loss_meter;
    LossMeter critic_2_loss_meter;
    LossMeter entropy_loss_meter;
    LossMeter episode_steps_meter;

    void check_train();

    void train(
        const torch::Tensor &batched_states, const torch::Tensor &batched_actions,
        const torch::Tensor &batched_rewards, const torch::Tensor &batched_done,
        const torch::Tensor &batched_next_state);

public:
    SoftActorCriticAgent(
        int seed, const std::vector<int64_t> &state_space, const std::vector<int64_t> &action_space,
        int hidden_size, int batch_size, float lr, float gamma, float tau, int replay_buffer_size);

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