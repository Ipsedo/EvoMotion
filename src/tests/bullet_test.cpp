//
// Created by samuel on 11/08/19.
//

#include "bullet_test.h"

#include "../core/env_list.h"
#include <iostream>

void test_bullet() {
    std::cout << "Bullet test" << std::endl;

    environment env_box = cartpole_env();

    while (env_box.will_draw) {
        env_step new_state = env_box.step(1.f / 60.f, torch::rand(1) - 0.5f);
        std::cout << new_state.state << std::endl;
    }

}