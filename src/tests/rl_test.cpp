//
// Created by samuel on 12/08/19.
//

#include "rl_test.h"
#include "../core/env_enum.h"
#include "../rl/agent_enum.h"
#include <chrono>
#include <sys/stat.h>


void train_reinforcement_learning(rl_train_info train_info) {
	std::cout << "Reinforcement learning train" << std::endl;

	// Init environment
	Environment *env = EnvEnum::from_str(train_info.env_name).get_env();

	// Init agent
	agent *ag = AgentEnum::from_str(train_info.agent_name)
			.get_agent(train_info.hidden_size, env->state_space(), env->action_space());

	if (env->is_action_discrete() != ag->is_discrete()) {
		std::cerr << "Agent (" << train_info.agent_name << ".discrete = " << ag->is_discrete() << ")"
		<< " is different of Environment (" << train_info.env_name << ".discrete = " << env->is_action_discrete() << ")."
		<< std::endl;
		exit(2);
	}

	std::cout << "Action space : " << env->action_space() << std::endl;
	std::cout << "State space : " << env->state_space() << std::endl;

	int consecutive_succes = 0;

	for (int i = 0; i < train_info.nb_episode; i++) {
		// Reset env and get initial step
		env_step state = env->reset();

		float cumulative_reward = 0.f;

		int episode_step = 0;

		// While episode is not finished and max step is not reached
		while (!state.done && episode_step < train_info.max_episode_step) {
			// Get agent's action from previous state
			auto act = ag->act(state.state, train_info.eps);

			// Perform agent's action on environment
			// And optionally display the env
			env_step new_state = env->step(1.f / 60.f, act, train_info.view);

			// Update agent
			ag->step(state.state, act, new_state.reward, new_state.state, new_state.done);

			// New state becomes actual state
			state = new_state;

			cumulative_reward += state.reward;

			// Decay epsilon threshold
			train_info.eps *= train_info.eps_decay;
			train_info.eps = train_info.eps < train_info.eps_min ? train_info.eps_min : train_info.eps;

			episode_step++;
		}

		// If episode max step is reached
		if (episode_step >= train_info.max_episode_step) consecutive_succes++;
		else consecutive_succes = 0;

		// If maximum step is reached 10 consecutive times
		if (consecutive_succes > train_info.max_consecutive_success) break;

		std::cout << std::fixed << std::setprecision(5)
		          << "Episode (" << std::setw(3) << i << ") : cumulative_reward = " << std::setw(9) << cumulative_reward
		          << " (" << std::setw(6) << cumulative_reward / episode_step << " $/step)"
		          << ", eps = " << std::setw(6) << train_info.eps << ", episode step : " << std::setw(3) << episode_step
		          << std::endl;
	}

	mkdir(train_info.out_model_folder.c_str(), 0755);

	ag->save(train_info.out_model_folder);

	delete env;
	delete ag;
}

void test_reinforcement_learning(rl_test_info test_info) {
	std::cout << "Reinforcement learning test" << std::endl;

	// Init environment
	Environment *env = EnvEnum::from_str(test_info.env_name).get_env();

	// Init agent
	agent *ag = AgentEnum::from_str(test_info.agent_name)
			.get_agent(test_info.hidden_size, env->state_space(), env->action_space());

	if (env->is_action_discrete() != ag->is_discrete()) {
		std::cerr << "Agent (" << test_info.agent_name << ".discrete = " << ag->is_discrete() << ")"
				  << " is different of Environment (" << test_info.env_name << ".discrete = " << env->is_action_discrete() << ")."
				  << std::endl;
		exit(2);
	}

	std::cout << "Action space : " << env->action_space() << std::endl;
	std::cout << "State space : " << env->state_space() << std::endl;

	ag->load(test_info.in_model_folder);

	// Test agent

	for (int i = 0; i < test_info.nb_episode; i++) {

		// Reset environment
		env_step state = env->reset();

		int step = 0;
		float cumulative_reward = 0.f;

		// Loop while episode is not finished
		while (!state.done) {
			// No epsilon greedy agent
			auto act = ag->act(state.state, 0.f);

			// Compute new state
			state = env->step(1.f / 60.f, act, test_info.view);

			cumulative_reward += state.reward;
			step++;
		}
		std::cout << "Episode (" << i << ") " << "Cumulative reward : " << cumulative_reward << " (" << cumulative_reward / step << " $/step)" << std::endl;
	}

	delete env;
	delete ag;
}
