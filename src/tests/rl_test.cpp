//
// Created by samuel on 12/08/19.
//

#include "rl_test.h"
#include "../core/env_list.h"
#include "../algos/dqn/deep_q_learning.h"
#include "../algos/ddpg/ddpg.h"
#include <chrono>

void test_reinforcement_learning() {
	std::cout << "Reinforcement learning test" << std::endl;

	// Init environment
	//Environment *cartpole_env = new CartPoleEnv(static_cast<int>(time(nullptr)));
	Environment *cartpole_env = new ContinuousCartPoleEnv(static_cast<int>(time(nullptr)));

	// Init agent
	//agent *ag = new dqn_agent(static_cast<int>(time(nullptr)), cartpole_env->state_space(), cartpole_env->action_space());
	agent *ag = new ddpg(static_cast<int>(time(nullptr)), cartpole_env->state_space(), cartpole_env->action_space());

	std::cout << "Action space : " << cartpole_env->action_space() << std::endl;
	std::cout << "State space : " << cartpole_env->state_space() << std::endl;

	int nb_episode = 300;
	int max_episode_step = 300;
	int consecutive_succes = 0;

	float eps = 1.f;
	float eps_decay = 0.9999f;
	float eps_min = 1e-2f;

	for (int i = 0; i < nb_episode; i++) {
		// Reset env and get initial step
		env_step state = cartpole_env->reset();

		float cumulative_reward = 0.f;

		int episode_step = 0;

		// While episode is not finished and max step is not reached
		while (!state.done && episode_step < max_episode_step) {
			// Get agent's action from previous state
			auto act = ag->act(state.state, eps);

			// Perform agent's action on environment
			env_step new_state = cartpole_env->step(1.f / 60.f, act, false);

			// Update agent
			ag->step(state.state, act, new_state.reward, new_state.state, new_state.done);

			// New state becomes actual state
			state = new_state;

			cumulative_reward += state.reward;

			// Decay epsilon threshold
			eps *= eps_decay;
			eps = eps < eps_min ? eps_min : eps;

			episode_step++;
		}

		// If episode max step is reached
		if (episode_step >= max_episode_step) consecutive_succes++;
		else consecutive_succes = 0;

		// If maximum step is reached 10 consecutive times
		if (consecutive_succes > 10) break;

		std::cout << std::fixed << std::setprecision(5)
		          << "Episode (" << std::setw(3) << i << ") : cumulative_reward = " << std::setw(9) << cumulative_reward
		          << ", eps = " << std::setw(6) << eps << ", episode step : " << std::setw(3) << episode_step
		          << std::endl;
	}

	// Test agent
	int nb_test = 100;

	for (int i = 0; i < nb_test; i++) {

		// Reset environment
		env_step state = cartpole_env->reset();

		std::cout << "New test episode" << std::endl;

		int step = 0;

		// Loop while episode is not finished
		while (!state.done) {
			// No epsilon greedy agent
			auto act = ag->act(state.state, 0.f);

			// Compute new state
			state = cartpole_env->step(1.f / 60.f, act, true);

			std::cout << "step : " << std::setw(4) << step << "\r" << std::flush;
			step++;
		}
		std::cout << std::endl;
	}

	delete cartpole_env;
	delete ag;
}
