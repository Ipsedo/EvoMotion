//
// Created by samuel on 05/12/24.
//

#ifndef PPO_GAE_LIQUID_H
#define PPO_GAE_LIQUID_H

#include <evo_motion_networks/agent.h>
#include <evo_motion_networks/networks/actor.h>
#include <evo_motion_networks/networks/critic.h>
#include <evo_motion_networks/replay_buffer.h>

class PpoGaeLiquidAgent final : public Agent {
public:
    PpoGaeLiquidAgent(
        int seed, const std::vector<int64_t> &state_space, const std::vector<int64_t> &action_space,
        int neuron_number, int unfolding_steps, float gamma, float lambda, float epsilon,
        float entropy_factor, float critic_loss_factor, int epoch, int batch_size, int train_every,
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
    std::shared_ptr<ActorLiquidModule> actor;
    std::shared_ptr<torch::optim::Optimizer> actor_optimizer;
    std::shared_ptr<CriticLiquidModule> critic;
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
    LiquidTrajectoryReplayBuffer<liquid_a2c_memory> replay_buffer;
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
        const torch::Tensor &batched_next_values, const torch::Tensor &batched_actor_x_t,
        const torch::Tensor &batched_critic_x_t);
};

#endif//PPO_GAE_LIQUID_H
