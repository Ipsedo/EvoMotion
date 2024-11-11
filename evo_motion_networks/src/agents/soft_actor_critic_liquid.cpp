//
// Created by samuel on 11/11/24.
//

#include <evo_motion_networks/agents/soft_actor_critic_liquid.h>
#include <evo_motion_networks/init.h>

/*
 * Torch Modules
 */

// Q-Network

/*
 * Agent
 */

torch::Tensor SoftActorCriticLiquidAgent::act(torch::Tensor state, float reward) {
    return torch::Tensor();
}

void SoftActorCriticLiquidAgent::done(torch::Tensor state, float reward) {}

void SoftActorCriticLiquidAgent::save(const std::string &output_folder_path) {}

void SoftActorCriticLiquidAgent::load(const std::string &input_folder_path) {}

std::vector<LossMeter> SoftActorCriticLiquidAgent::get_metrics() {
    return std::vector<LossMeter>();
}

void SoftActorCriticLiquidAgent::to(torch::DeviceType device) {}

void SoftActorCriticLiquidAgent::set_eval(bool eval) {}

int SoftActorCriticLiquidAgent::count_parameters() { return 0; }
