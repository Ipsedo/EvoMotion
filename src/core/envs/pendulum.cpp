//
// Created by samuel on 10/09/19.
//

#include "pendulum.h"

// All params (physical params, reward, state features, actions) are picked from :
// https://github.com/openai/gym/wiki/Pendulum-v0

PendulumParams::PendulumParams()
		: pendule_mass(1.f), hinge_push_force(5e-1f), max_step(200), reset_nb_frame(1) {}

PendulumEnv::PendulumEnv(int seed) : rd_gen(seed), rd_uni(0.f, 1.f),
	PendulumParams(), Environment(renderer(1920, 1080), init_items()), episode_step(0), last_action(0.f) {
	m_engine.m_world->addConstraint(hinge);
}

torch::IntArrayRef PendulumEnv::action_space() {
	return torch::IntArrayRef({1});
}

torch::IntArrayRef PendulumEnv::state_space() {
	return torch::IntArrayRef({3});
}

std::vector<item> PendulumEnv::init_items() {
	pendule_pos_y = 0.f, pendule_pos_z = 10.f;
	float pendule_height = 1.f, pendule_width = 0.1f;
	float base_scale = 0.1f, base_pos_y = pendule_pos_y + pendule_height, base_pos_z = pendule_pos_z;

	item pendule = create_item_box(glm::vec3(0.f, pendule_pos_y, pendule_pos_z),
			glm::mat4(1.f), glm::vec3(pendule_width, pendule_height, pendule_width), pendule_mass);

	item base = create_item_box(glm::vec3(0.f, base_pos_y, base_pos_z),
			glm::mat4(1.f), glm::vec3(base_scale), 0.f);

	pendule_rg = pendule.m_rg_body;

	btVector3 axis(0.f, 0.f ,1.f);

	hinge = new btHingeConstraint(*pendule_rg, *base.m_rg_body,
			btVector3(0.f, pendule_height, 0.f), btVector3(0.f, 0.f, -base_scale),
			axis, axis, true);
	hinge->setEnabled(true);
	hinge->setLimit(1.f, -1.f);

	pendule_rg->setIgnoreCollisionCheck(base.m_rg_body, true);

	return std::vector<item>({pendule, base});
}

void PendulumEnv::act(torch::Tensor action) {

	last_action = action[0].item().toFloat();
	//pendule_rg->applyTorque(btVector3(0.f, 0.f, last_action * hinge_push_force));
	pendule_rg->applyTorqueImpulse(btVector3(0.f, 0.f, last_action * hinge_push_force));

	episode_step++;
}

env_step PendulumEnv::compute_new_state() {
	float theta = hinge->getHingeAngle();
	theta += theta > 0.f ? -M_PI : M_PI;

	float cos_theta = cos(theta);
	float sin_theta = sin(theta);

    // Theta dot : derivative of angular velocity
	float theta_dt = pendule_rg->getAngularVelocity().z();

	float reward = -float((pow(theta, 2.) + 1e-1 * pow(theta_dt, 2.) + 1e-3 * pow(last_action, 2.)));

	torch::Tensor state = torch::tensor({cos_theta, sin_theta, theta_dt});

	return env_step{state, reward, episode_step >= max_step};
}

env_step PendulumEnv::reset_engine() {
	episode_step = 0;

	btTransform tr_pendule_reset;
	tr_pendule_reset.setIdentity();
	tr_pendule_reset.setOrigin(btVector3(0.f, pendule_pos_y, pendule_pos_z));

	pendule_rg->setWorldTransform(tr_pendule_reset);
	pendule_rg->getMotionState()->setWorldTransform(tr_pendule_reset);

	pendule_rg->setLinearVelocity(btVector3(0.f, 0.f, 0.f));
	pendule_rg->setAngularVelocity(btVector3(0.f, 0.f, 0.f));
	pendule_rg->clearForces();

	last_action = 0.f;
	for (int i = 0; i < reset_nb_frame; i++) {
		last_action = rd_uni(rd_gen) * 2.f - 1.f;
		pendule_rg->applyTorqueImpulse(btVector3(0.f, 0.f, last_action * hinge_push_force));
		m_engine.m_world->stepSimulation(1.f / 60.f);
	}

    return compute_new_state();
}

