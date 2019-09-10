//
// Created by samuel on 19/08/19.
//

#include "ddpg.h"

/////////////
// Actor
/////////////

actor::actor(torch::IntArrayRef state_space, torch::IntArrayRef action_space) {
	l1 = register_module("l1", torch::nn::Linear(state_space[0], 24));
	l2 = register_module("l2", torch::nn::Linear(24, 24));
	l3 = register_module("l3", torch::nn::Linear(24, action_space[0]));
}

torch::Tensor actor::forward(torch::Tensor input) {
	auto out_l1 = torch::relu(l1->forward(input));
	auto out_l2 = torch::relu(l2->forward(out_l1));
	auto pred = torch::tanh(l3->forward(out_l2));
	return pred;
}

/////////////
// Critic
/////////////

critic::critic(torch::IntArrayRef state_space, torch::IntArrayRef action_space)  {
	l1 = register_module("l1", torch::nn::Linear(state_space[0], 24));
	l2 = register_module("l2", torch::nn::Linear(24 + action_space[0], 24));
	l3 = register_module("l3", torch::nn::Linear(24, 1));
}

torch::Tensor critic::forward(std::tuple<torch::Tensor,torch::Tensor> state_action) {
	auto state = std::get<0>(state_action);
	auto action = std::get<1>(state_action);

	auto out_l1 = torch::relu(l1->forward(state));
	auto out_l2 = torch::relu(l2->forward(torch::cat({out_l1, action}, -1)));
	auto pred = l3->forward(out_l2);
	return pred;
}

///////////////////////////////////////////
// Deep Deterministic Policy Gradient
///////////////////////////////////////////

ddpg::ddpg(int seed, torch::IntArrayRef state_space, torch::IntArrayRef action_space) :
		agent(state_space, action_space, 3000),
		m_actor(state_space, action_space), m_actor_target(state_space, action_space),
		m_critic(state_space, action_space), m_critic_target(state_space, action_space),
		actor_optim(torch::optim::Adam(m_actor.parameters(), 1e-3)),
		critic_optim(torch::optim::Adam(m_critic.parameters(), 1e-3)),
		batch_size(16), update_every(4), current_step(0), gamma(0.95), tau(1e-3),
		rd_gen(seed), rd_uni(0.f, 1.f) {
	// Hard copy : actor target <- actor
	m_actor_target.l1->weight = m_actor.l1->weight.clone();
	m_actor_target.l1->bias = m_actor.l1->bias.clone();

	m_actor_target.l2->weight = m_actor.l2->weight.clone();
	m_actor_target.l2->bias = m_actor.l2->bias.clone();

	m_actor_target.l3->weight = m_actor.l3->weight.clone();
	m_actor_target.l3->bias = m_actor.l3->bias.clone();

	// Hard copy : critic target <- critic
	m_critic_target.l1->weight = m_critic.l1->weight.clone();
	m_critic_target.l1->bias = m_critic.l1->bias.clone();

	m_critic_target.l2->weight = m_critic.l2->weight.clone();
	m_critic_target.l2->bias = m_critic.l2->bias.clone();

	m_critic_target.l3->weight = m_critic.l3->weight.clone();
	m_critic_target.l3->bias = m_critic.l3->bias.clone();

}

void ddpg::step(torch::Tensor state, torch::Tensor action, float reward, torch::Tensor next_state, bool done) {
	memory_buffer.add(state, action, reward, next_state, done);

	// Step counter
	current_step = (current_step + 1) % update_every;

	// Update every n step
	if (current_step == 0) {
		// If memory size is sufficient
		if (memory_buffer.mem.size() > batch_size) {
			// Pick randomly samples
			auto t = memory_buffer.sample(batch_size);

			torch::Tensor sample_states = std::get<0>(t);
			torch::Tensor sample_actions = std::get<1>(t);
			torch::Tensor sample_rewards = std::get<2>(t);
			torch::Tensor sample_next_states = std::get<3>(t);
			torch::Tensor sample_dones = std::get<4>(t);

			// Learn on those samples
			learn(sample_states, sample_actions, sample_rewards, sample_next_states, sample_dones);
		}
	}
}

at::Tensor ddpg::act(torch::Tensor state, float eps) {
	torch::NoGradGuard no_grad;

	if (rd_uni(rd_gen) > eps) return m_actor.forward(state.unsqueeze(0)).squeeze(0);

	return torch::rand(m_action_space) * 2.f - 1.f;
}

void ddpg::learn(torch::Tensor states, torch::Tensor actions, torch::Tensor rewards, torch::Tensor next_states,
                 torch::Tensor dones) {
	torch::Tensor next_q_values = m_critic_target.forward(
			std::tuple<torch::Tensor, torch::Tensor>(next_states, m_actor_target.forward(next_states)));

	torch::Tensor target_q = rewards + gamma * dones.to(torch::kFloat) * next_q_values;

	torch::Tensor q = m_critic.forward(std::tuple<torch::Tensor, torch::Tensor>(states, actions));

	torch::Tensor loss_v = torch::mse_loss(q, target_q.detach());

	critic_optim.zero_grad();
	loss_v.backward();
	critic_optim.step();

	torch::Tensor policy_loss = -m_critic.forward(
			std::tuple<torch::Tensor, torch::Tensor>(states, m_actor.forward(states))).mean();

	actor_optim.zero_grad();
	policy_loss.backward();
	actor_optim.step();

	// Soft Update
	m_actor_target.l1->weight.copy_(tau * m_actor.l1->weight + (1 - tau) * m_actor.l1->weight);
	m_actor_target.l2->weight.copy_(tau * m_actor.l2->weight + (1 - tau) * m_actor.l2->weight);
	m_actor_target.l3->weight.copy_(tau * m_actor.l3->weight + (1 - tau) * m_actor.l3->weight);

	m_actor_target.l1->bias.copy_(tau * m_actor.l1->bias + (1 - tau) * m_actor.l1->bias);
	m_actor_target.l2->bias.copy_(tau * m_actor.l2->bias + (1 - tau) * m_actor.l2->bias);
	m_actor_target.l3->bias.copy_(tau * m_actor.l3->bias + (1 - tau) * m_actor.l3->bias);

	m_critic_target.l1->weight.copy_(tau * m_critic.l1->weight + (1 - tau) * m_critic.l1->weight);
	m_critic_target.l2->weight.copy_(tau * m_critic.l2->weight + (1 - tau) * m_critic.l2->weight);
	m_critic_target.l3->weight.copy_(tau * m_critic.l3->weight + (1 - tau) * m_critic.l3->weight);

	m_critic_target.l1->bias.copy_(tau * m_critic.l1->bias + (1 - tau) * m_critic.l1->bias);
	m_critic_target.l2->bias.copy_(tau * m_critic.l2->bias + (1 - tau) * m_critic.l2->bias);
	m_critic_target.l3->bias.copy_(tau * m_critic.l3->bias + (1 - tau) * m_critic.l3->bias);
}
