//
// Created by samuel on 19/08/19.
//

#ifndef EVOMOTION_DDPG_H
#define EVOMOTION_DDPG_H


#include "agent.h"
#include <torch/torch.h>

struct actor : torch::nn::Module {

};

struct critic : torch::nn::Module {

};

struct ddpg : agent {
	ddpg(torch::IntArrayRef state_space, torch::IntArrayRef action_space);

	void step(torch::Tensor state, torch::Tensor action, float reward, torch::Tensor next_state, bool done) override;

	at::Tensor act(torch::Tensor state, float eps) override;
};


#endif //EVOMOTION_DDPG_H
