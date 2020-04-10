//
// Created by samuel on 11/08/19.
//

#ifndef EVOMOTION_DEEP_Q_LEARNING_H
#define EVOMOTION_DEEP_Q_LEARNING_H

#include <torch/torch.h>
#include <random>
#include "agent.h"

/**
 * Basic Q-Network
 * 3 linear (fully-connected) layers
 */
struct q_network : torch::nn::Module {

	q_network(torch::IntArrayRef state_space, torch::IntArrayRef action_space, int hidden_size);

	torch::Tensor forward(torch::Tensor input);

	torch::nn::Linear l1{nullptr};
	torch::nn::Linear l2{nullptr};
	torch::nn::Linear l3{nullptr};

};

// https://github.com/udacity/deep-reinforcement-learning/blob/master/dqn/solution/dqn_agent.py
struct dqn_agent : agent {

	// Random
	std::default_random_engine rd_gen;
	std::uniform_real_distribution<float> rd_uni;

	// LibTorch stuff
	q_network local_q_network;
	q_network target_q_network;

	torch::optim::Adam optimizer;

	bool is_cuda;

	// Algo hyper-params
	int batch_size;
	float gamma;
	float tau;
	int update_every;
	int idx_step;

	dqn_agent(int seed, torch::IntArrayRef state_space, torch::IntArrayRef action_space, int hidden_size);

	void step(torch::Tensor state, torch::Tensor action, float reward,
	          torch::Tensor next_state, bool done) override;

	torch::Tensor act(torch::Tensor state, float eps) override;

	/**
	 * Perform forward and backward on Q-Network
	 * @param states The states
	 * @param actions The actions
	 * @param rewards The rewards
	 * @param next_states The next states
	 * @param dones The dones indicator
	 */
	void learn(torch::Tensor states, torch::Tensor actions, torch::Tensor rewards,
	           torch::Tensor next_states, torch::Tensor dones);

	void save(std::string out_folder_path) override;

	void load(std::string input_folder_path) override;

	bool is_discrete() override;

    void cuda() override;

    void cpu() override;
};

#endif //EVOMOTION_DEEP_Q_LEARNING_H
