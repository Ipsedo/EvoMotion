//
// Created by samuel on 12/08/19.
//

#include "rl_test.h"
#include "../core/env_list.h"

void test_reinforcement_learning() {
	std::cout << "Reinforcement learning test" << std::endl;

	environment cartpole_env = create_cartpole_env();

	std::cout << "Action space : " << cartpole_env.m_actions_sizes << std::endl;
	std::cout << "State space : " << cartpole_env.m_state_sizes << std::endl;

	int nb_try = 20;

	bool done = false;

	for (int i = 0; i < nb_try; i++) {
		while (!done) {
			//bool will_draw = i < 5 ? true : (i > 15 ? true : cartpole_env.m_renderer.m_is_on); // test
			bool will_draw = true;
			env_step new_state = cartpole_env.step(1.f / 60.f, torch::rand(1) * 2.f - 1.f, will_draw);
			//std::cout << new_state.state << std::endl;
			done = new_state.done;
			std::cout << "Episode : " << i << ", reward = " << new_state.reward << std::endl;
		}
		cartpole_env.reset();
		done = false;
		std::cout << "reset" << std::endl;
	}
}
