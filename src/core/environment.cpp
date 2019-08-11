//
// Created by samuel on 11/08/19.
//

#include "environment.h"

environment::environment(renderer renderer, std::vector<item> items, bool will_draw)
    : m_items(items), m_engine(items), m_renderer(renderer), will_draw(will_draw) {
}

void environment::step(float delta) {
    m_engine.step(delta);

    if (will_draw) {
        will_draw = m_renderer.draw(delta, m_items);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000 / 60));
    }
}
