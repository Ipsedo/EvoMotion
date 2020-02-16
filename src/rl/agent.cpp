//
// Created by samuel on 13/08/19.
//

#include "agent.h"
#include "deep_q_learning.h"
#include "ddpg.h"
#include <random>

replay_buffer::replay_buffer(int max_size, unsigned long seed) :
		max_size(max_size), mem(),
		rd_gen(seed),
		rd_uni(0.f, 1.f) {}

void replay_buffer::add(torch::Tensor state, torch::Tensor action, float reward, torch::Tensor next_state, bool done) {
	memory m{std::move(state), std::move(action), reward, std::move(next_state), done};

	mem.push_back(m);

	// Last first added element if memory size exceed limit
	if (mem.size() > max_size)
		mem.pop_front();
}

std::tuple<torch::Tensor, torch::Tensor, torch::Tensor, torch::Tensor, torch::Tensor>
replay_buffer::sample(int batch_size) {
	std::vector<memory> tmp;

	if (batch_size < 1) {
		std::cout << "Need sample batch size > 1 !" << std::endl;
		exit(1);
	}

	std::vector<torch::Tensor> state_list, action_list, reward_list, new_state_list, done_list;

	for (int i = 0; i < batch_size; i++) {
		// Randomly pick one sample
		int idx = int(floor(rd_uni(rd_gen) * mem.size()));
		auto sample = mem[idx];

		state_list.push_back(sample.state);
		action_list.push_back(sample.action);
		reward_list.push_back(torch::tensor(sample.reward));
		new_state_list.push_back(sample.next_state);
		done_list.push_back(torch::tensor({sample.done ? 1.f : 0.f}));
	}

	// Construct Tensor containing the samples
	torch::Tensor states = torch::stack(state_list, 0);
	torch::Tensor actions = torch::stack(action_list, 0);
	torch::Tensor rewards = torch::stack(reward_list, 0);
	torch::Tensor new_states = torch::stack(new_state_list, 0);
	torch::Tensor dones = torch::stack(done_list, 0);

	// Return the tuple containing the samples
	return std::tuple<torch::Tensor, torch::Tensor, torch::Tensor, torch::Tensor, torch::Tensor>(
			states, actions, rewards, new_states, dones);
}

// Agent
agent::agent(torch::IntArrayRef state_space, torch::IntArrayRef action_space, int buffer_size) :
		m_state_space(state_space), m_action_space(action_space),
		memory_buffer(buffer_size, static_cast<unsigned long>(time(nullptr))) {}

agent::~agent() {}

// Random agent
random_agent::random_agent(torch::IntArrayRef state_space,
                           torch::IntArrayRef action_space) : agent(state_space, action_space, 10) {
}

void random_agent::step(torch::Tensor state, torch::Tensor action, float reward, torch::Tensor next_state,
                        bool done) {
	// Do nothing for random agent;
	memory_buffer.add(state, action, reward, next_state, done);

	auto t = memory_buffer.sample(10);
}

torch::Tensor random_agent::act(torch::Tensor state, float eps) {
	return torch::rand(m_action_space);
}

void random_agent::save(std::string out_folder_path) {

}

void random_agent::load(std::string input_folder_path) {

}

bool random_agent::is_discrete() {
	return false;
}
