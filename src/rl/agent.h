//
// Created by samuel on 13/08/19.
//

#ifndef EVOMOTION_AGENT_H
#define EVOMOTION_AGENT_H

#include <torch/torch.h>
#include <deque>
#include <random>

/**
 * A struct representing one sample
 */
struct memory {
	torch::Tensor state;
	torch::Tensor action;
	float reward;
	torch::Tensor next_state;
	bool done;
};

struct replay_buffer {
	// State memory
	std::deque<memory> mem;

	// The memory max size
	int max_size;

	// Random stuff
	std::mt19937 rd_gen;
	std::uniform_real_distribution<float> rd_uni;

	replay_buffer(int max_size, unsigned long seed);

	/**
	 * Add step to memory
	 * @param state The old state
	 * @param action The action performed on old state
	 * @param reward The reward gained with the action on old state
	 * @param next_state The next state
	 * @param done If next state is episode end
	 */
	void add(torch::Tensor state, torch::Tensor action, float reward, torch::Tensor next_state, bool done);

	/**
	 * Return std::tuple : < state, action, reward, next_state, done >
	 * @param batch_size The batch size, first size of the Tensors retruned
	 * @return The tuple containing the samples
	 */
	std::tuple<torch::Tensor, torch::Tensor, torch::Tensor, torch::Tensor, torch::Tensor>
	sample(int batch_size);
};


struct agent {
	// The agent state and action space
	torch::IntArrayRef m_state_space, m_action_space;

	// The agent replay memory
	replay_buffer memory_buffer;

	agent(torch::IntArrayRef state_space, torch::IntArrayRef action_space, int buffer_size);

	/**
	 * Step the agent (update, learn, etc.)
	 * @param state The old state
	 * @param action The action to perform with the old state
	 * @param reward The reward gained when perform "action" on "state"
	 * @param next_state The next state when "action" is perform on "state"
	 * @param done If the episode is done int the state "next_state"
	 */
	virtual void step(torch::Tensor state, torch::Tensor action, float reward, torch::Tensor next_state, bool done) = 0;

	/**
	 * Get agent action on the current state
	 * @param state The current state
	 * @param eps The epsilon greedy factor
	 * @return The choosed action
	 */
	virtual torch::Tensor act(torch::Tensor state, float eps) = 0;

	/**
	 *
	 * @param out_path
	 */
	virtual void save(std::string out_folder_path) = 0;

	/**
	 *
	 * @param input_folder_path
	 */
	virtual void load(std::string input_folder_path) = 0;

	virtual bool is_discrete() = 0;

	virtual void cuda() = 0;
	virtual void cpu() = 0;

	virtual ~agent();
};


struct random_agent : agent {
	random_agent(torch::IntArrayRef state_space, torch::IntArrayRef action_space);

	void step(torch::Tensor state, torch::Tensor action, float reward, torch::Tensor next_state, bool done) override;

	torch::Tensor act(torch::Tensor state, float eps) override;

    void save(std::string out_folder_path) override;

    void load(std::string input_folder_path) override;

    bool is_discrete() override;

	void cuda() override;

	void cpu() override;
};

#endif //EVOMOTION_AGENT_H
