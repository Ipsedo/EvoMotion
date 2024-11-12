//
// Created by samuel on 29/03/24.
//

#ifndef EVO_MOTION_ACTOR_CRITIC_LIQUID_H
#define EVO_MOTION_ACTOR_CRITIC_LIQUID_H

#include "../networks/liquid.h"
#include "./actor_critic.h"
#include "./soft_actor_critic.h"

// Agent

class ActorCriticLiquidAgent : public Agent {
private:
    std::shared_ptr<ActorLiquidModule> actor;
    std::shared_ptr<torch::optim::Adam> actor_optimizer;
    std::shared_ptr<CriticLiquidModule> critic;
    std::shared_ptr<torch::optim::Adam> critic_optimizer;

    float gamma;
    float entropy_start_factor;
    float entropy_end_factor;
    long entropy_steps;

    torch::DeviceType curr_device;

    int batch_size;
    LiquidA2cReplayBuffer replay_buffer;

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
        const torch::Tensor &batched_next_state, const liquid_a2c_step_memory &curr_memory,
        const liquid_a2c_step_memory &next_memory);

public:
    ActorCriticLiquidAgent(
        int seed, const std::vector<int64_t> &state_space, const std::vector<int64_t> &action_space,
        int neuron_number, int batch_size, float lr, float gamma, float entropy_start_factor,
        float entropy_end_factor, long entropy_steps, int unfolding_steps, int replay_buffer_size);

    void done(torch::Tensor state, float reward) override;

    torch::Tensor act(torch::Tensor state, float reward) override;

    void save(const std::string &output_folder_path) override;

    void load(const std::string &input_folder_path) override;

    std::vector<LossMeter> get_metrics() override;

    void to(torch::DeviceType device) override;

    void set_eval(bool eval) override;

    int count_parameters() override;
};

/*class SoftActorCriticLiquidAgent : public SoftActorCriticAgent {
public:
    SoftActorCriticLiquidAgent(
        int seed, const std::vector<int64_t> &state_space, const std::vector<int64_t> &action_space,
        int hidden_size, int batch_size, float lr, float gamma, float tau, int unfolding_steps);

    void done(torch::Tensor state, float reward) override;
};*/

#endif//EVO_MOTION_ACTOR_CRITIC_LIQUID_H