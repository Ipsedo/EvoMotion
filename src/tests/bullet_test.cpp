//
// Created by samuel on 11/08/19.
//

#include "bullet_test.h"

#include "../core/envs/test.h"
#include <iostream>

void test_bullet() {
	std::cout << "Bullet test" << std::endl;

	Environment *env = new TestEnv(1234);

	while (env->is_renderer_on()) {
		env_step new_state = env->step(1.f / 60.f, torch::rand(1) * 2.f - 1.f, true);
		std::cout << new_state.state << std::endl;
	}
}