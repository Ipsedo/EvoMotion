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
#include <evo_motion_networks/metrics.h>
#include <evo_motion_networks/networks/actor.h>
#include <evo_motion_networks/networks/critic.h>
#include <evo_motion_networks/replay_buffer.h>

// Agent

class ActorCriticAgent : public Agent {
private:
    std::shared_ptr<ActorModule> actor;
    std::shared_ptr<torch::optim::Optimizer> actor_optimizer;

    std::shared_ptr<CriticModule> critic;
    std::shared_ptr<torch::optim::Optimizer> critic_optimizer;

    float gamma;
    float entropy_start_factor;
    float entropy_end_factor;
    long entropy_steps;

    torch::DeviceType curr_device;

    int batch_size;
    ReplayBuffer replay_buffer;

    LossMeter policy_loss_meter;
    LossMeter entropy_meter;
    LossMeter critic_loss_meter;
    LossMeter episode_steps_meter;

    int curr_episode_step;
    long curr_train_step;
    long global_curr_step;

    void check_train();

    void train(
        const torch::Tensor &batched_states, const torch::Tensor &batched_actions,
        const torch::Tensor &batched_rewards, const torch::Tensor &batched_done,
        const torch::Tensor &batched_next_state);

public:
    ActorCriticAgent(
        int seed, const std::vector<int64_t> &state_space, const std::vector<int64_t> &action_space,
        int hidden_size, int batch_size, float actor_lr, float critic_lr, float gamma, float entropy_start_factor,
        float entropy_end_factor, long entropy_steps, int replay_buffer_size);

    torch::Tensor act(torch::Tensor state, float reward) override;

    void done(torch::Tensor state, float reward) override;

    void save(const std::string &output_folder_path) override;

    void load(const std::string &input_folder_path) override;

    std::vector<LossMeter> get_metrics() override;

    void to(torch::DeviceType device) override;

    void set_eval(bool eval) override;

    int count_parameters() override;
};

#endif//EVO_MOTION_ACTOR_CRITIC_H