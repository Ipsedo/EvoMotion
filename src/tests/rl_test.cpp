//
// Created by samuel on 12/08/19.
//

#include "rl_test.h"
#include "../core/env_list.h"
#include "../algos/rl/deep_q_learning.h"
//#include "../algos/rl/ddpg.h"
#include <chrono>

void test_reinforcement_learning() {
	std::cout << "Reinforcement learning test" << std::endl;

	Environment *cartpole_env = new CartPoleEnv(static_cast<int>(time(nullptr)));

	agent *ag = new dqn_agent(static_cast<int>(time(nullptr)),
	                          cartpole_env->state_space(), cartpole_env->action_space());

	std::cout << "Action space : " << cartpole_env->action_space() << std::endl;
	std::cout << "State space : " << cartpole_env->state_space() << std::endl;

	int nb_episode = 100;
	int max_episode_step = 300;
	int consecutive_succes = 0;

	float eps = 1.f;
	float eps_decay = 0.9995f;
	float eps_min = 1e-2f;

	int step = 0;

	for (int i = 0; i < nb_episode; i++) {
		env_step state = cartpole_env->reset();

		float cumulative_reward = 0.f;

		int episode_step = 0;

		while (!state.done && episode_step < max_episode_step) {
			auto act = ag->act(state.state, eps);

			env_step new_state = cartpole_env->step(1.f / 60.f, act, false);

			ag->step(state.state, act, new_state.reward, new_state.state, new_state.done);

			state = new_state;

			cumulative_reward += state.reward;

			eps *= eps_decay;
			//eps = 1.f / sqrt(step + 1.f);
			eps = eps < eps_min ? eps_min : eps;

			episode_step++;
			step++;
		}

		if (episode_step >= max_episode_step) consecutive_succes++;
		else consecutive_succes = 0;

		if (consecutive_succes > 10) break;

		std::cout << std::fixed << std::setprecision(5)
		          << "Episode (" << std::setw(3) << i << ") : cumulative_reward = " << std::setw(9) << cumulative_reward
		          << ", eps = " << std::setw(6) << eps << ", episode step : " << std::setw(3) << episode_step
		          << std::endl;
	}

	int nb_test = 100;

	for (int i = 0; i < nb_test; i++) {

		env_step state = cartpole_env->reset();

		while (!state.done) {
			auto act = ag->act(state.state, 0.f);

			env_step new_state = cartpole_env->step(1.f / 60.f, act, true);

			//ag.step(state.state, act, new_state.reward, new_state.state, new_state.done);

			state = new_state;

		}
	}

	delete cartpole_env;
}
