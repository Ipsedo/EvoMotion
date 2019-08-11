//
// Created by samuel on 11/08/19.
//

#include "environment.h"

environment::environment(renderer renderer, std::vector<item> items, bool will_draw,
        std::function<void(torch::Tensor, std::vector<item>)> apply_action,
        std::function<env_step(std::vector<item>)> process_env)
    : m_items(items), m_engine(items), m_renderer(renderer), will_draw(will_draw),
    m_apply_action(std::move(apply_action)),
    m_get_state(std::move(process_env)) {
}

env_step environment::step(float delta, torch::Tensor action) {
    m_apply_action(std::move(action), m_items);
    m_engine.step(delta);

    if (will_draw) {
        will_draw = m_renderer.draw(delta, m_items);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000 / 60));
    }

    return m_get_state(m_items);
}
