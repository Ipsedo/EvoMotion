//
// Created by samuel on 11/08/19.
//

#include "bullet_test.h"

#include "../core/env_list.h"
#include <iostream>

void test_bullet() {
    std::cout << "Bullet test" << std::endl;

    environment env_box = create_test_env();

    while (env_box.m_renderer.m_is_on) {
        env_step new_state = env_box.step(1.f / 60.f, torch::rand(1) * 2.f - 1.f, true);
        std::cout << new_state.state << std::endl;
    }
}