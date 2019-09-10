//
// Created by samuel on 10/09/19.
//

#include "pendulum.h"


PendulumParams::PendulumParams()
		: pendule_mass(1.f), hinge_force(2e2f), hinge_max_speed(5.f), max_step(200) {}

// https://github.com/openai/gym/wiki/Pendulum-v0
PendulumEnv::PendulumEnv(int seed) : rd_gen(seed), rd_uni(0.f, 1.f),
	PendulumParams(), Environment(renderer(1920 / 2, 1080 / 2), init_items()) {
	m_engine.m_world->addConstraint(hinge);
}

torch::IntArrayRef PendulumEnv::action_space() {
	return torch::IntArrayRef({1});
}

torch::IntArrayRef PendulumEnv::state_space() {
	return torch::IntArrayRef({3});
}

std::vector<item> PendulumEnv::init_items() {
	float pendule_pos_y = 0.f, pendule_pos_z = 10.f, pendule_height = 2.f, pendule_width = 0.1f;
	float base_scale = 0.1f, base_pos_y = 0.f, base_pos_z = 10.f + pendule_width + base_scale;

	item base = create_item_box(glm::vec3(0.f, pendule_pos_y, pendule_pos_z),
			glm::mat4(1.f), glm::vec3(pendule_width, pendule_height, pendule_width), pendule_mass);

	item pendule = create_item_box(glm::vec3(0.f, base_pos_y, base_pos_z), glm::mat4(1.f), glm::vec3(base_scale), 0.f);

	pendule_rg = pendule.m_rg_body;

	btVector3 axis(0.f, 0.f ,1.f);

	hinge = new btHingeConstraint(*pendule_rg, *base.m_rg_body,
			btVector3(0.f, -pendule_height, 0.f), btVector3(0.f, 0.f, -base_scale),
			axis, axis, true);
	// TODO enable motor, motor force
	hinge->setEnabled(true);
	hinge->setMaxMotorImpulse(hinge_force);

	pendule_rg->setIgnoreCollisionCheck(base.m_rg_body, true);

	return std::vector<item>({pendule, base});
}

void PendulumEnv::act(torch::Tensor action) {
	hinge->setMotorTargetVelocity(action[0].item().toFloat() * hinge_max_speed);
}

env_step PendulumEnv::compute_new_state() {
	return env_step{torch::zeros({3}), 0.f, false};
}

env_step PendulumEnv::reset_engine() {
	return env_step{torch::zeros({3}), 0.f, false};
}

