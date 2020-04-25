//
// Created by samuel on 19/08/19.
//

#ifndef EVOMOTION_DDPG_H
#define EVOMOTION_DDPG_H


#include "agent.h"
#include <torch/torch.h>

struct actor : torch::nn::Module {
	actor(torch::IntArrayRef state_space, torch::IntArrayRef action_space, int hidden_size);

	torch::Tensor forward(torch::Tensor input);

	torch::nn::Linear l1{nullptr};
	torch::nn::Linear l2{nullptr};
	torch::nn::Linear l3{nullptr};
};

struct critic : torch::nn::Module {
	critic(torch::IntArrayRef state_space, torch::IntArrayRef action_space, int hidden_size);

	torch::Tensor forward(std::tuple<torch::Tensor,torch::Tensor> state_action);

	torch::nn::Linear l1{nullptr};
	torch::nn::Linear l2{nullptr};
	torch::nn::Linear l3{nullptr};
};

// https://github.com/ghliu/pytorch-ddpg/blob/master/ddpg.py
struct ddpg : agent {

	// Random stuff
	std::default_random_engine rd_gen;
	std::uniform_real_distribution<float> rd_uni;

	// Torch stuff
	actor m_actor;
	critic m_critic;

	actor m_actor_target;
	critic m_critic_target;

	torch::optim::Adam actor_optim;
	torch::optim::Adam critic_optim;

	float tau, gamma;
	long unsigned int batch_size;
	int update_every, current_step;
	bool is_cuda;

	ddpg(int seed, torch::IntArrayRef state_space, torch::IntArrayRef action_space, int hidden_size);

	void step(torch::Tensor state, torch::Tensor action, float reward, torch::Tensor next_state, bool done) override;

	at::Tensor act(torch::Tensor state, float eps) override;

	void learn(torch::Tensor states, torch::Tensor actions, torch::Tensor rewards,
	           torch::Tensor next_states, torch::Tensor dones);

	void save(std::string out_folder_path) override;

	void load(std::string input_folder_path) override;

    bool is_discrete() override;

    void cuda() override;

    void cpu() override;
};


#endif //EVOMOTION_DDPG_H
