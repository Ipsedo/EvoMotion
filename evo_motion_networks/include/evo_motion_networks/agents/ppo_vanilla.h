//
// Created by samuel on 21/11/24.
//

#ifndef EVO_MOTION_PPO_VANILLA_H
#define EVO_MOTION_PPO_VANILLA_H

#include <evo_motion_networks/agent.h>
#include <evo_motion_networks/metrics.h>
#include <evo_motion_networks/networks/actor.h>
#include <evo_motion_networks/networks/critic.h>
#include <evo_motion_networks/replay_buffer.h>

class PpoVanillaAgent final : public Agent {
public:
    PpoVanillaAgent(
        int seed, const std::vector<int64_t> &state_space, const std::vector<int64_t> &action_space,
        int hidden_size, float gamma, float epsilon, float entropy_factor, float critic_loss_factor,
        int epoch, int batch_size, float learning_rate);

    torch::Tensor act(torch::Tensor state, float reward) override;

    void done(torch::Tensor state, float reward) override;

    void save(const std::string &output_folder_path) override;

    void load(const std::string &input_folder_path) override;

    std::vector<LossMeter> get_metrics() override;

    void to(torch::DeviceType device) override;

    void set_eval(bool eval) override;

    int count_parameters() override;

private:
    std::shared_ptr<ActorModule> actor;
    std::shared_ptr<torch::optim::Optimizer> actor_optimizer;
    std::shared_ptr<CriticModule> critic;
    std::shared_ptr<torch::optim::Optimizer> critic_optimizer;

    float gamma;
    float epsilon;
    int epoch;

    float entropy_factor;
    float critic_loss_factor;

    long curr_train_step;
    long curr_episode_step;
    long global_curr_step;

    int batch_size;
    ReplayBuffer replay_buffer;
    int train_every;

    LossMeter actor_loss_meter;
    LossMeter critic_loss_meter;
    LossMeter episode_steps_meter;

    torch::DeviceType curr_device;

    void check_train();
    void train(
        const torch::Tensor &batched_states, const torch::Tensor &batched_actions,
        const torch::Tensor &batched_rewards, const torch::Tensor &batched_done,
        const torch::Tensor &batched_next_state);
};

#endif//EVO_MOTION_PPO_VANILLA_H
