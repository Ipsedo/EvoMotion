//
// Created by samuel on 12/08/19.
//

#include "rl_test.h"
#include "../core/env_list.h"

void test_reinforcement_learning() {
	std::cout << "Bullet test" << std::endl;

	environment env_box = cartpole_env(true);

	bool done = false;
	while (!done) {
		env_step new_state = env_box.step(1.f / 60.f, torch::rand(1) * 2.f - 1.f);
		std::cout << new_state.state << std::endl;
		done = new_state.done;
	}
}
