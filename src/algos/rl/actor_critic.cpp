//
// Created by samuel on 16/08/19.
//

#include "actor_critic.h"


actor_critic::actor_critic(torch::IntArrayRef state_space, torch::IntArrayRef action_space) :
	agent(state_space, action_space, 10000) {

}

void actor_critic::step(torch::Tensor state, torch::Tensor action, float reward, torch::Tensor next_state, bool done) {

}

torch::Tensor actor_critic::act(torch::Tensor state, float eps) {
	return torch::rand(m_action_space);
}
