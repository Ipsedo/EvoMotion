//
// Created by samuel on 13/08/19.
//

#include "cartpole.h"

CartPoleEnvParams::CartPoleEnvParams() : slider_speed(4.f), chariot_push_force(10.f),
                                         limit_angle(float(M_PI * 0.25)), reset_frame_nb(1),
                                         chariot_mass(1.f), pendule_mass(1e-1f), slider_force(2e2f) {

}

CartPoleEnv::CartPoleEnv(int seed) : rd_gen(seed), rd_uni(0.f, 1.f), CartPoleEnvParams(),
                                     Environment(renderer(1920 / 2, 1080 / 2), init_cartpole()) {
	m_engine.m_world->addConstraint(slider);
	m_engine.m_world->addConstraint(hinge);
}

std::vector<item> CartPoleEnv::init_cartpole() {

	float base_height = 2.f, base_pos = -4.f;

	float pendule_height = 0.7f, pendule_width = 0.1f, pendule_offset = pendule_height / 4.f;

	float chariot_height = 0.25f, chariot_width = 0.5f;

	chariot_pos = base_pos + base_height + chariot_height;
	pendule_pos = chariot_pos + chariot_height + pendule_height - pendule_offset;

	item base = create_item_box(glm::vec3(0.f, base_pos, 10.f), glm::mat4(1.f), glm::vec3(10.f, base_height, 10.f),
	                            0.f);

	item chariot = create_item_box(glm::vec3(0.f, chariot_pos, 10.f),
	                               glm::mat4(1.f),
	                               glm::vec3(chariot_width, chariot_height, chariot_width), chariot_mass);
	item pendule = create_item_box(glm::vec3(0.f, pendule_pos, 10.f),
	                               glm::mat4(1.f),
	                               glm::vec3(pendule_width, pendule_height, pendule_width), pendule_mass);

	std::vector<item> items{base, chariot, pendule};

	// For slider between chariot and base
	btTransform tr_base;
	tr_base.setIdentity();
	tr_base.setOrigin(btVector3(0.f, base_height, 0.f));

	btTransform tr_chariot;
	tr_chariot.setIdentity();
	tr_chariot.setOrigin(btVector3(0.f, -chariot_height, 0.f));

	slider = new btSliderConstraint(*base.m_rg_body, *chariot.m_rg_body, tr_base, tr_chariot, true);

	slider->setEnabled(true);
	slider->setPoweredLinMotor(true);
	slider->setMaxLinMotorForce(slider_force);
	slider->setTargetLinMotorVelocity(0.f);
	slider->setLowerLinLimit(-10.f);
	slider->setUpperLinLimit(10.f);

	// For hinge between pendule and chariot
	btVector3 axis(0.f, 0.f, 1.f);
	hinge = new btHingeConstraint(*chariot.m_rg_body, *pendule.m_rg_body,
	                              btVector3(0.f, chariot_height, 0.f),
	                              btVector3(0.f, -pendule_height + pendule_offset, 0.f),
	                              axis, axis, true);

	base_rg = base.m_rg_body;
	chariot_rg = chariot.m_rg_body;
	pendule_rg = pendule.m_rg_body;

	/*btTransform center_of_mass_tr;
	center_of_mass_tr.setIdentity();
	center_of_mass_tr.setOrigin(btVector3(0.f, -pendule_height / 2.f, 0.f));
	pendule_rg->setCenterOfMassTransform(center_of_mass_tr);*/

	chariot_rg->setIgnoreCollisionCheck(pendule_rg, true);
	base_rg->setIgnoreCollisionCheck(chariot_rg, true);
	base_rg->setIgnoreCollisionCheck(pendule_rg, true);

	return items;
}

torch::IntArrayRef CartPoleEnv::action_space() {
	return torch::IntArrayRef({3});
}

torch::IntArrayRef CartPoleEnv::state_space() {
	return torch::IntArrayRef({4});
}

CartPoleEnv::~CartPoleEnv() {

}

void CartPoleEnv::act(torch::Tensor action) {
	int act_idx = action.argmax(-1).item().toInt();

	float speed;
	if (act_idx == 0) speed = 1.f;
	else if (act_idx == 1) speed = -1.f;
	else speed = 0.f;

	speed *= slider_speed;

	slider->setTargetLinMotorVelocity(speed);
	//chariot_rg->applyCentralImpulse(btVector3(speed, 0.f, 0.f));
	//chariot_rg->setLinearVelocity(btVector3(speed, 0.f, 0.f));
}

env_step CartPoleEnv::compute_new_state() {
	float pos = chariot_rg->getWorldTransform().getOrigin().x();
	float vel = chariot_rg->getLinearVelocity().x();

	float ang = pendule_rg->getWorldTransform().getRotation().getAngle();
	float ang_vel = pendule_rg->getAngularVelocity().z();

	torch::Tensor state = torch::tensor({pos, vel, ang, ang_vel});

	bool done = pos > 8.f || pos < -8.f || ang > limit_angle || ang < -limit_angle;
	float reward = abs(limit_angle - abs(ang) / limit_angle);
	//float reward = done ? 0.f : 1.f;

	//std::cout << state << std::endl;
	env_step new_state{state, reward, done};
	return new_state;
}

env_step CartPoleEnv::reset_engine() {
	m_engine.m_world->removeRigidBody(chariot_rg);
	m_engine.m_world->removeRigidBody(pendule_rg);
	m_engine.m_world->removeConstraint(slider);
	m_engine.m_world->removeConstraint(hinge);

	//slider->setTargetLinMotorVelocity(0.f);

	btTransform tr_chariot_reset;
	tr_chariot_reset.setIdentity();
	tr_chariot_reset.setOrigin(btVector3(0.f, chariot_pos, 10.f));

	chariot_rg->setWorldTransform(tr_chariot_reset);
	chariot_rg->getMotionState()->setWorldTransform(tr_chariot_reset);

	chariot_rg->setLinearVelocity(btVector3(0.f, 0.f, 0.f));
	chariot_rg->setAngularVelocity(btVector3(0.f, 0.f, 0.f));
	chariot_rg->clearForces();

	// Pendule
	btTransform tr_pendule_reset;
	tr_pendule_reset.setIdentity();
	tr_pendule_reset.setOrigin(btVector3(0.f, pendule_pos, 10.f));

	pendule_rg->setWorldTransform(tr_pendule_reset);
	pendule_rg->getMotionState()->setWorldTransform(tr_pendule_reset);

	pendule_rg->setLinearVelocity(btVector3(0.f, 0.f, 0.f));
	pendule_rg->setAngularVelocity(btVector3(0.f, 0.f, 0.f));
	pendule_rg->clearForces();

	m_engine.m_world->addRigidBody(chariot_rg);
	m_engine.m_world->addRigidBody(pendule_rg);
	m_engine.m_world->addConstraint(slider);
	m_engine.m_world->addConstraint(hinge);

	float rand_force = (rd_uni(rd_gen) * 0.5f + 0.5f) * chariot_push_force;
	rand_force = rd_uni(rd_gen) > 0.5f ? rand_force : -rand_force;
	chariot_rg->applyCentralImpulse(btVector3(rand_force, 0.f, 0.f));

	for (int i = 0; i < reset_frame_nb; i++)
		m_engine.step(1.f / 60.f);

	return compute_new_state();
}
