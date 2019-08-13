//
// Created by samuel on 12/08/19.
//

#include "rl_test.h"
#include "../core/env_list.h"

void test_reinforcement_learning() {
	std::cout << "Reinforcement learning test" << std::endl;

	environment cartpole_env = create_cartpole_env();

	int nb_try = 10;

	bool done = false;

	for (int i = 0; i < nb_try; i++) {
		while (!done) {
			env_step new_state = cartpole_env.step(1.f / 60.f, torch::rand(1) * 2.f - 1.f, i > 5);
			std::cout << new_state.state << std::endl;
			done = new_state.done;
		}
		cartpole_env.reset();
		done = false;
		std::cout << "reset" << std::endl;
	}
}
