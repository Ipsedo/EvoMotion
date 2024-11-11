//
// Created by samuel on 11/11/24.
//

#ifndef EVO_MOTION_SOFT_ACTOR_CRITIC_LIQUID_H
#define EVO_MOTION_SOFT_ACTOR_CRITIC_LIQUID_H

#include <evo_motion_networks/agent.h>
#include <evo_motion_networks/agents/actor_critic.h>
#include <evo_motion_networks/agents/actor_critic_liquid.h>
#include <evo_motion_networks/agents/soft_actor_critic.h>

/*
 * Agent
 */

class SoftActorCriticLiquidAgent : public Agent {
public:
    torch::Tensor act(torch::Tensor state, float reward) override;

    void done(torch::Tensor state, float reward) override;

    void save(const std::string &output_folder_path) override;

    void load(const std::string &input_folder_path) override;

    std::vector<LossMeter> get_metrics() override;

    void to(torch::DeviceType device) override;

    void set_eval(bool eval) override;

    int count_parameters() override;
};

#endif//EVO_MOTION_SOFT_ACTOR_CRITIC_LIQUID_H
