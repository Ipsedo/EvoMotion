//
// Created by samuel on 16/08/19.
//

#ifndef EVOMOTION_ACTOR_CRITIC_H
#define EVOMOTION_ACTOR_CRITIC_H

#include "agent.h"


struct actor_critic : agent {
	actor_critic(torch::IntArrayRef state_space, torch::IntArrayRef action_space);
	void step(torch::Tensor state, torch::Tensor action, float reward,
			  torch::Tensor next_state, bool done) override;
	torch::Tensor act(torch::Tensor state, float eps) override;
};


#endif //EVOMOTION_ACTOR_CRITIC_H
