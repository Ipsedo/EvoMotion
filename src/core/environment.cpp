//
// Created by samuel on 11/08/19.
//

#include "environment.h"

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

