//
// Created by samuel on 19/08/19.
//

#include "ddpg.h"

/////////////
// Actor
/////////////

actor::actor(torch::IntArrayRef state_space, torch::IntArrayRef action_space, int hidden_size) {
	l1 = register_module("l1", torch::nn::Linear(state_space[0], hidden_size));
	l2 = register_module("l2", torch::nn::Linear(hidden_size, hidden_size));
	l3 = register_module("l3", torch::nn::Linear(hidden_size, action_space[0]));
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

critic::critic(torch::IntArrayRef state_space, torch::IntArrayRef action_space, int hidden_size)  {
	l1 = register_module("l1", torch::nn::Linear(state_space[0], hidden_size));
	l2 = register_module("l2", torch::nn::Linear(hidden_size + action_space[0], hidden_size));
	l3 = register_module("l3", torch::nn::Linear(hidden_size, 1));
}

torch::Tensor critic::forward(std::tuple<torch::Tensor,torch::Tensor> state_action) {
	auto state = std::get<0>(state_action);
	auto action = std::get<1>(state_action);

	auto out_l1 = torch::relu(l1->forward(state));
	auto out_l2 = torch::relu(l2->forward(torch::cat({out_l1, action}, -1)));
	auto pred = torch::tanh(l3->forward(out_l2));
	return pred;
}

///////////////////////////////////////////
// Deep Deterministic Policy Gradient
///////////////////////////////////////////

ddpg::ddpg(int seed, torch::IntArrayRef state_space, torch::IntArrayRef action_space, int hidden_size) :
		agent(state_space, action_space, 100000),
		m_actor(state_space, action_space, hidden_size), m_actor_target(state_space, action_space, hidden_size),
		m_critic(state_space, action_space, hidden_size), m_critic_target(state_space, action_space, hidden_size),
		actor_optim(torch::optim::Adam(m_actor.parameters(), 1e-4)),
		critic_optim(torch::optim::Adam(m_critic.parameters(), 1e-3)),
		batch_size(64), update_every(4), current_step(0), gamma(0.95), tau(1e-3),
		rd_gen(seed), rd_uni(0.f, 1.f), is_cuda(false) {
	// Hard copy : actor target <- actor
	m_actor_target.l1->weight.data().copy_(m_actor.l1->weight.data());
	m_actor_target.l1->bias.data().copy_(m_actor.l1->bias.data());

	m_actor_target.l2->weight.data().copy_(m_actor.l2->weight.data());
	m_actor_target.l2->bias.data().copy_(m_actor.l2->bias.data());

	m_actor_target.l3->weight.data().copy_(m_actor.l3->weight.data());
	m_actor_target.l3->bias.data().copy_(m_actor.l3->bias.data());

	// Hard copy : critic target <- critic
	m_critic_target.l1->weight.data().copy_(m_critic.l1->weight.data());
	m_critic_target.l1->bias.data().copy_(m_critic.l1->bias.data());

	m_critic_target.l2->weight.data().copy_(m_critic.l2->weight.data());
	m_critic_target.l2->bias.data().copy_(m_critic.l2->bias.data());

	m_critic_target.l3->weight.data().copy_(m_critic.l3->weight.data());
	m_critic_target.l3->bias.data().copy_(m_critic.l3->bias.data());
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

			if (is_cuda) {
				sample_states = sample_states.to(torch::Device(torch::kCUDA));
				sample_actions = sample_actions.to(torch::Device(torch::kCUDA));
				sample_rewards = sample_rewards.to(torch::Device(torch::kCUDA));
				sample_next_states = sample_next_states.to(torch::Device(torch::kCUDA));
				sample_dones = sample_dones.to(torch::Device(torch::kCUDA));
			}

			// Learn on those samples
			learn(sample_states, sample_actions, sample_rewards, sample_next_states, sample_dones);
		}
	}
}

at::Tensor ddpg::act(torch::Tensor state, float eps) {
	torch::NoGradGuard no_grad;

	if (rd_uni(rd_gen) > eps)  {
        if (is_cuda) state = state.to(torch::Device(torch::kCUDA));

	    auto act = m_actor.forward(state.unsqueeze(0)).squeeze(0);

        if (is_cuda) act = act.to(torch::Device(torch::kCPU));

        return act;
	}

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
	m_actor_target.l1->weight.data().copy_(tau * m_actor.l1->weight.data() + (1 - tau) * m_actor.l1->weight.data());
	m_actor_target.l2->weight.data().copy_(tau * m_actor.l2->weight.data() + (1 - tau) * m_actor.l2->weight.data());
	m_actor_target.l3->weight.data().copy_(tau * m_actor.l3->weight.data() + (1 - tau) * m_actor.l3->weight.data());

	m_actor_target.l1->bias.data().copy_(tau * m_actor.l1->bias.data() + (1 - tau) * m_actor.l1->bias.data());
	m_actor_target.l2->bias.data().copy_(tau * m_actor.l2->bias.data() + (1 - tau) * m_actor.l2->bias.data());
	m_actor_target.l3->bias.data().copy_(tau * m_actor.l3->bias.data() + (1 - tau) * m_actor.l3->bias.data());

	m_critic_target.l1->weight.data().copy_(tau * m_critic.l1->weight.data() + (1 - tau) * m_critic.l1->weight.data());
	m_critic_target.l2->weight.data().copy_(tau * m_critic.l2->weight.data() + (1 - tau) * m_critic.l2->weight.data());
	m_critic_target.l3->weight.data().copy_(tau * m_critic.l3->weight.data() + (1 - tau) * m_critic.l3->weight.data());

	m_critic_target.l1->bias.data().copy_(tau * m_critic.l1->bias.data() + (1 - tau) * m_critic.l1->bias.data());
	m_critic_target.l2->bias.data().copy_(tau * m_critic.l2->bias.data() + (1 - tau) * m_critic.l2->bias.data());
	m_critic_target.l3->bias.data().copy_(tau * m_critic.l3->bias.data() + (1 - tau) * m_critic.l3->bias.data());
}

void ddpg::save(std::string out_folder_path) {
	// Network files
    std::string actor_target_file = out_folder_path + "/" + "actor_target.th";
    std::string critic_target_file = out_folder_path + "/" + "critic_target.th";

    std::string actor_file = out_folder_path + "/" + "actor.th";
    std::string critic_file = out_folder_path + "/" + "critic.th";

	torch::serialize::OutputArchive actor_output_archive;
	torch::serialize::OutputArchive critic_output_archive;
	torch::serialize::OutputArchive actor_target_output_archive;
	torch::serialize::OutputArchive critic_target_output_archive;

    // Save networks
    m_actor.save(actor_output_archive);
    m_critic.save(critic_output_archive);
    m_actor_target.save(actor_target_output_archive);
    m_critic_target.save(critic_target_output_archive);

	actor_output_archive.save_to(actor_file);
	critic_output_archive.save_to(critic_file);
	actor_target_output_archive.save_to(actor_target_file);
	critic_target_output_archive.save_to(critic_target_file);

    // Optimizer files
	std::string critic_optim_file = out_folder_path + "/" + "critic_optim.th";
	std::string actor_optim_file = out_folder_path + "/" + "actor_optim.th";

	torch::serialize::OutputArchive critic_optim_ouput_archive;
	torch::serialize::OutputArchive actor_optim_ouput_archive;

	// Save optimizers
	actor_optim.save(actor_optim_ouput_archive);
	critic_optim.save(critic_optim_ouput_archive);

	critic_optim_ouput_archive.save_to(critic_optim_file);
	actor_optim_ouput_archive.save_to(actor_optim_file);
}

void ddpg::load(std::string input_folder_path) {
	std::string actor_target_file = input_folder_path + "/" + "actor_target.th";
	std::string critic_target_file = input_folder_path + "/" + "critic_target.th";

	std::string actor_file = input_folder_path + "/" + "actor.th";
	std::string critic_file = input_folder_path + "/" + "critic.th";

	torch::serialize::InputArchive actor_input_archive;
	torch::serialize::InputArchive critic_input_archive;
	torch::serialize::InputArchive actor_target_input_archive;
	torch::serialize::InputArchive critic_target_input_archive;

	actor_input_archive.load_from(actor_file);
	critic_input_archive.load_from(critic_file);
	actor_target_input_archive.load_from(actor_target_file);
	critic_target_input_archive.load_from(critic_target_file);

	m_actor.load(actor_input_archive);
	m_critic.load(critic_input_archive);
	m_actor_target.load(actor_target_input_archive);
	m_critic_target.load(critic_target_input_archive);

	std::string critic_optim_file = input_folder_path + "/" + "critic_optim.th";
	std::string actor_optim_file = input_folder_path + "/" + "actor_optim.th";

	torch::serialize::InputArchive actor_optim_input_archive;
	torch::serialize::InputArchive critic_optim_input_archive;

	actor_optim_input_archive.load_from(actor_optim_file);
	critic_optim_input_archive.load_from(critic_optim_file);

	actor_optim.load(actor_optim_input_archive);
	critic_optim.load(critic_optim_input_archive);
}

bool ddpg::is_discrete() {
	return false;
}

void ddpg::cuda() {
    is_cuda = true;

	m_actor.to(torch::Device(torch::kCUDA));
	m_critic.to(torch::Device(torch::kCUDA));

	m_actor_target.to(torch::Device(torch::kCUDA));
	m_critic_target.to(torch::Device(torch::kCUDA));
}

void ddpg::cpu() {
    is_cuda = false;

	m_actor.to(torch::Device(torch::kCPU));
	m_critic.to(torch::Device(torch::kCPU));

	m_actor_target.to(torch::Device(torch::kCPU));
	m_critic_target.to(torch::Device(torch::kCPU));
}
