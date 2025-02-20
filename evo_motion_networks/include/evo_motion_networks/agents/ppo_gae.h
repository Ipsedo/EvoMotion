//
// Created by samuel on 13/11/24.
//

#ifndef EVO_MOTION_PPO_GAE_H
#define EVO_MOTION_PPO_GAE_H

#include <evo_motion_networks/agent.h>
#include <evo_motion_networks/metrics.h>
#include <evo_motion_networks/networks/actor.h>
#include <evo_motion_networks/networks/critic.h>
#include <evo_motion_networks/replay_buffer.h>

class PpoGaeAgent final : public Agent {
public:
    PpoGaeAgent(
        int seed, const std::vector<int64_t> &state_space, const std::vector<int64_t> &action_space,
        int hidden_size, float gamma, float lambda, float epsilon, float entropy_factor,
        float critic_loss_factor, int epoch, int batch_size, int train_every,
        int replay_buffer_size, float learning_rate, float clip_grad_norm);

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
    float lambda;
    float epsilon;
    int epoch;

    float entropy_factor;
    float critic_loss_factor;
    float clip_grad_norm;

    long curr_train_step;
    long curr_episode_step;
    long global_curr_step;

    int batch_size;
    TrajectoryReplayBuffer replay_buffer;
    int train_every;

    LossMeter actor_loss_meter;
    LossMeter critic_loss_meter;
    LossMeter episode_steps_meter;

    torch::DeviceType curr_device;

    void check_train();
    void train(
        const torch::Tensor &batched_states, const torch::Tensor &batched_actions,
        const torch::Tensor &batched_rewards, const torch::Tensor &batched_done,
        const torch::Tensor &batched_log_prob, const torch::Tensor &batched_curr_values,
        const torch::Tensor &batched_next_values);
};

#endif//EVO_MOTION_PPO_GAE_H
