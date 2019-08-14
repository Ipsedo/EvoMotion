//
// Created by samuel on 12/08/19.
//

#include "rl_test.h"
#include "../core/env_list.h"
#include "../algos/rl/deep_q_learning.h"

void test_reinforcement_learning() {
	std::cout << "Reinforcement learning test" << std::endl;

	environment cartpole_env = create_cartpole_env();

    auto ag = dqn_agent(cartpole_env.m_state_sizes, cartpole_env.m_actions_sizes);

	std::cout << "Action space : " << cartpole_env.m_actions_sizes << std::endl;
	std::cout << "State space : " << cartpole_env.m_state_sizes << std::endl;

	int nb_episode = 300;
	int max_episode_step = 300;
	int consecutive_succes = 0;

	float eps = 0.5f;
	float eps_decay = 0.9995f;
	float eps_min = 5e-2f;

	int step = 0;

	for (int i = 0; i < nb_episode; i++) {
		env_step state = cartpole_env.get_first_state();

		float cumulative_reward = 0.f;

		int episode_step = 0;

		while (!state.done && episode_step < max_episode_step) {
			auto act = ag.act(state.state, eps);

			env_step new_state = cartpole_env.step(1.f / 60.f, act, false);

			ag.step(state.state, act, new_state.reward, new_state.state, new_state.done);

			state = new_state;

			cumulative_reward += state.reward;

			eps *= eps_decay;
			eps = eps < eps_min ? eps_min : eps;

			episode_step++;
			step++;
		}

		if (episode_step >= max_episode_step) consecutive_succes++;
		else consecutive_succes = 0;

		if (consecutive_succes > 10) { cartpole_env.reset(); break; }

		cartpole_env.reset();
		std::cout << std::fixed << std::setprecision(5)
		<< "Episode (" << std::setw(3) << i << ") : cumulative_reward = " << std::setw(9) << cumulative_reward
		<< ", eps = " << std::setw(9) << eps << std::endl;
	}

	int nb_test = 0;

	for (int i = 0; i < nb_test; i++) {

		env_step state = cartpole_env.get_first_state();

		while (!state.done) {
			auto act = ag.act(state.state, 0.f);

			env_step new_state = cartpole_env.step(1.f / 60.f, act, true);

			//ag.step(state.state, act, new_state.reward, new_state.state, new_state.done);

			state = new_state;

		}

		cartpole_env.reset();
	}
}
