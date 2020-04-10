//
// Created by samuel on 11/08/19.
//

#include "environment.h"


Environment::Environment(renderer renderer, std::vector<item> items) :
		m_items(std::move(items)), m_renderer(renderer), m_engine(items), m_step(0) {}

const env_step Environment::step(float delta, torch::Tensor action, bool will_draw) {
	// Perform action
	act(std::move(action));

	// Step bullet physical world
	m_engine.step(delta);

	// If drawing is needed
	if (will_draw) {
		// If renderer is not initialized, init it
		if (!m_renderer.m_is_on)
			m_renderer.init();
		// Draw frame
		m_renderer.draw(delta, m_items);
		// Sleep for 60 Hz
		std::this_thread::sleep_for(std::chrono::milliseconds(1000 / 60));
	}

	m_step += 1;

	// Return new state
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
	/*for (auto i = m_items.size() - 1; i >= 0; i--) {
		item it = m_items[i];
		// Only remove graphical object
		// Bullet's rigidbody is deleted in engine struct
		delete it.m_obj_mtl_vbo;
	}*/
}
