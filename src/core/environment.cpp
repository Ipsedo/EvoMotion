//
// Created by samuel on 11/08/19.
//

#include "environment.h"

/*environment::environment(renderer renderer, std::vector<item> items,
        torch::IntArrayRef action_sizes, torch::IntArrayRef state_sizes,
        std::function<void(torch::Tensor, std::vector<item>)> act_fun,
        std::function<env_step(std::vector<item>)> step_fun,
        std::function<void(std::vector<item>)> reset_fun)
    : m_items(items), m_engine(items), m_renderer(renderer), m_step(0),
    m_actions_sizes(action_sizes), m_state_sizes(state_sizes),
    m_act_fun(std::move(act_fun)),
    m_step_fun(std::move(step_fun)),
    m_reset_fun(std::move(reset_fun))
    { }

env_step environment::get_first_state() {
    return m_step_fun(m_items);
}

env_step environment::step(float delta, torch::Tensor action, bool will_draw) {
    m_act_fun(std::move(action), m_items);
    m_engine.step(delta);

    if (will_draw) {
        if (!m_renderer.m_is_on)
            m_renderer.init();
        m_renderer.draw(delta, m_items);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000 / 60));
    }

    m_step += 1;

    return m_step_fun(m_items);
}

void environment::reset() {
    m_reset_fun(m_items);
}*/


Environment::Environment(renderer renderer, std::vector<item> items) :
    m_items(std::move(items)), m_renderer(renderer), m_engine(items), m_step(0) {

}

const env_step Environment::step(float delta, torch::Tensor action, bool will_draw) {
    act(action);

    m_engine.step(delta);

    if (will_draw) {
        if (!m_renderer.m_is_on)
            m_renderer.init();
        m_renderer.draw(delta, m_items);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000 / 60));
    }

    m_step += 1;

    return compute_new_state();
}

bool Environment::is_renderer_on() {
    return m_renderer.m_is_on;
}

const env_step Environment::reset() {
    m_step = 0;
    return reset_engine();
}

Environment::~Environment() {
    for (auto i = m_items.size() - 1; i >= 0; i--) {
        item it = m_items[i];
        delete it.m_obj_mtl_vbo;
    }
}

