//
// Created by samuel on 10/09/19.
//

#include "pendulum.h"


PendulumParams::PendulumParams(float pendule_mass, float hinge_force, float hinge_max_speed, int max_step)
		: pendule_mass(pendule_mass), hinge_force(hinge_force), hinge_max_speed(hinge_max_speed), max_step(max_step) {}

// https://github.com/openai/gym/wiki/Pendulum-v0
Pendulum::Pendulum(int seed) : rd_gen(seed), rd_uni(0.f, 1.f),
	PendulumParams(1.f, 2e2f, 5.f, 200), Environment(renderer(1920 / 2, 1080 / 2), init_items()) {
	m_engine.m_world->addConstraint(hinge);
}

torch::IntArrayRef Pendulum::action_space() {
	return torch::IntArrayRef({1});
}

torch::IntArrayRef Pendulum::state_space() {
	return torch::IntArrayRef({3});
}

std::vector<item> Pendulum::init_items() {


	float pendule_pos_y = 0.f, pendule_pos_z = 0.f, pendule_height = 4.f, pendule_width = 0.2f;
	float base_scale = 3.f, base_pos_y = 0.f, base_pos_z = 0.f + pendule_width + base_scale;

	item base = create_item_box(glm::vec3(0.f, pendule_pos_y, pendule_pos_z),
			glm::mat4(1.f), glm::vec3(pendule_width, pendule_height, pendule_width), pendule_mass);

	item pendule = create_item_box(glm::vec3(0.f, base_pos_y, base_pos_z), glm::mat4(1.f), glm::vec3(base_scale), 0.f);

	pendule_rg = pendule.m_rg_body;

	btVector3 axis(0.f, 0.f ,1.f);

	hinge = new btHingeConstraint(*pendule_rg, *base.m_rg_body,
			btVector3(0.f, -pendule_height, 0.f), btVector3(0.f, 0.f, -base_scale),
			axis, axis, true);
	// TODO enable motor, motor force

	return std::vector<item>({pendule, base});
}

void Pendulum::act(torch::Tensor action) {

}

env_step Pendulum::compute_new_state() {
	return env_step();
}

env_step Pendulum::reset_engine() {
	return env_step();
}

