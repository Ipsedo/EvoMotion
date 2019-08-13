//
// Created by samuel on 12/08/19.
//

#include "rl_test.h"
#include "../core/env_list.h"
#include "../algos/rl/agent.h"

void test_reinforcement_learning() {
	std::cout << "Reinforcement learning test" << std::endl;

	environment cartpole_env = create_cartpole_env();

    auto ag = dqn_agent(cartpole_env.m_state_sizes, cartpole_env.m_actions_sizes);

	std::cout << "Action space : " << cartpole_env.m_actions_sizes << std::endl;
	std::cout << "State space : " << cartpole_env.m_state_sizes << std::endl;

	int nb_try = 200;

	for (int i = 0; i < nb_try; i++) {
		env_step state = cartpole_env.get_first_state();

		float cumulative_reward = 0.f;

		int step = 0;

		//float eps = float(1. / (log(i + 1.) / log(2.f)));
		float eps = 1e-3f;

		while (!state.done && step < 300) {
			auto act = ag.act(state.state, eps);

			env_step new_state = cartpole_env.step(1.f / 60.f, act, true);

			ag.step(state.state, act, new_state.reward, new_state.state, new_state.done);

			state = new_state;

			cumulative_reward += state.reward;

			step++;
		}
		cartpole_env.reset();
		std::cout << "Episode (" << i << ") : cumulative_reward = " << cumulative_reward << std::endl;
	}

	env_step state = cartpole_env.get_first_state();

	while (!state.done) {
		auto act = ag.act(state.state, 1e-3f);

		env_step new_state = cartpole_env.step(1.f / 60.f, act, true);

		//ag.step(state.state, act, new_state.reward, new_state.state, new_state.done);

		state = new_state;

	}
}
