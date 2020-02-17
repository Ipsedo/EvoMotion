//
// Created by samuel on 10/09/19.
//

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "pendulum.h"

// All params (physical params, reward, state features, actions) are picked from :
// https://github.com/openai/gym/wiki/Pendulum-v0

PendulumParams::PendulumParams()
		: pendule_mass(1.f), hinge_push_force(1e-2f), max_step(400), reset_nb_frame(2),
		  pendule_pos_y(0.f), pendule_pos_z(10.f), pendule_height(0.5f),
		  base_pos_y(pendule_pos_y + pendule_height), base_pos_z(pendule_pos_z) {}

PendulumEnv::PendulumEnv(long seed) : rd_gen(seed), rd_uni(0.f, 1.f),
									  PendulumParams(), Environment(renderer(1920, 1080), init_items()),
									  episode_step(0), last_action(0.f), last_theta_dt(0.f) {
	m_engine.m_world->addConstraint(hinge, true);
}

torch::IntArrayRef PendulumEnv::action_space() {
	return torch::IntArrayRef({1});
}

torch::IntArrayRef PendulumEnv::state_space() {
	return torch::IntArrayRef({5});
}

std::vector<item> PendulumEnv::init_items() {
	float pendule_width = 0.1f;
	float base_scale = 0.1f;

	item pendule = create_item_box(glm::vec3(0.f, pendule_pos_y, pendule_pos_z),
			glm::mat4(1.f), glm::vec3(pendule_width, pendule_height, pendule_width), pendule_mass);

	item base = create_item_box(glm::vec3(0.f, base_pos_y, base_pos_z),
			glm::mat4(1.f), glm::vec3(base_scale), 0.f);

	base_rg = base.m_rg_body;
	pendule_rg = pendule.m_rg_body;

	base_rg->setActivationState(DISABLE_DEACTIVATION);
	pendule_rg->setActivationState(DISABLE_DEACTIVATION);

	btVector3 axis(0.f, 0.f ,1.f);

	hinge = new btHingeConstraint(*pendule_rg, *base_rg,
			btVector3(0.f, pendule_height, 0.f), btVector3(0.f, 0.f, -base_scale),
			axis, axis, true);
	hinge->setEnabled(true);
	hinge->setLimit(1.f, -1.f);

	return std::vector<item>({pendule, base});
}

void PendulumEnv::act(torch::Tensor action) {

	last_action = action[0].item().toFloat();

	//pendule_rg->applyTorque(btVector3(0.f, 0.f, last_action * hinge_push_force));

	//float theta = hinge->getHingeAngle();
	//btVector3 impulse(cos(theta), sin(theta), 0);
	//pendule_rg->applyImpulse(hinge_push_force * last_action * impulse, btVector3(0.f, pendule_pos_y + pendule_height, pendule_pos_z));

	pendule_rg->applyTorqueImpulse(btVector3(0.f, 0.f, last_action * hinge_push_force));

	//hinge->setMaxMotorImpulse(last_action * hinge_push_force);

    /*btScalar tmp[16];
    btTransform tr;
    pendule_rg->getMotionState()->getWorldTransform(tr);
    tr.getOpenGLMatrix(tmp);

    glm::vec4 ortho = glm::make_mat4(tmp) * glm::vec4(1.f, 0.f, 0.f, 0.f);*/

    //pendule_rg->applyImpulse(btVector3(last_action * hinge_push_force, 0, 0), btVector3(0.f, -pendule_height, 0));

	episode_step++;
}

env_step PendulumEnv::compute_new_state() {
	float theta = hinge->getHingeAngle();

	float cos_theta = cos(theta);
	float sin_theta = sin(theta);

	theta = (theta > 0 ? 1.f : -1.f ) * (float(M_PI) + (theta > 0 ? -theta : theta));

    // Theta dot : derivative of angular velocity
	float theta_dt = pendule_rg->getAngularVelocity().z();

	// last_action * hinge_push_force ?
	//std::cout << theta << " " << pow(theta, 2.) << ", " << theta_dt << " " << pow(theta_dt, 2.) << ", " << last_action << " " << pow(last_action, 2.) << std::endl;
	float reward = - float(pow(theta, 2.) + 1e-1 * pow(theta_dt, 2.) + 1e-3 * pow(last_action * 2, 2.));
	//float reward = - float(pow(theta, 2.) + pow(theta_dt, 2.) + pow(last_action, 2.));
	//float reward = -float((pow(theta, 2.) + 1e-3 * pow(theta_dt, 2.) + 1e-5 * pow(last_action, 2.)));
	//float reward = -abs(theta) - 1e-1f * abs(theta_dt);

	float torque = pendule_rg->getTotalTorque().z();

	torch::Tensor state = torch::tensor({cos_theta, sin_theta, theta_dt, theta_dt - last_theta_dt, torque});

	last_theta_dt = theta_dt;

	return env_step{state, reward, episode_step >= max_step};
}

env_step PendulumEnv::reset_engine() {

	// Reset objects
	m_engine.m_world->removeConstraint(hinge);
	m_engine.m_world->removeRigidBody(base_rg);
	m_engine.m_world->removeRigidBody(pendule_rg);

	// Pendule
	btTransform tr_pendule_reset;
	tr_pendule_reset.setIdentity();
	tr_pendule_reset.setOrigin(btVector3(0.f, pendule_pos_y, pendule_pos_z));

	btTransform trans;
	trans.setIdentity();
	trans.setOrigin(btVector3(0.f, -pendule_height, 0.f));

	btTransform rot;
	rot.setIdentity();

	btQuaternion q;
	float angle = rd_uni(rd_gen) * float(M_PI) * 2.f - float(M_PI);
	q.setRotation(btVector3(0.f, 0.f, 1.f), angle);

	rot.setRotation(q);

	pendule_rg->clearForces();
	btVector3 zeroVector(0.f, 0.f, 0.f);
	pendule_rg->setLinearVelocity(zeroVector);
	pendule_rg->setAngularVelocity(zeroVector);
	pendule_rg->setWorldTransform(trans.inverse() * rot * trans * tr_pendule_reset);

	// Base
	btTransform tr_base_reset;
	tr_base_reset.setIdentity();
	tr_base_reset.setOrigin(btVector3(0.f, base_pos_y, base_pos_z));

	base_rg->setCenterOfMassTransform(tr_base_reset);
	//pendule_rg->getMotionState()->setWorldTransform(tr_base_reset);

	base_rg->clearForces();
	base_rg->setLinearVelocity(zeroVector);
	base_rg->setAngularVelocity(zeroVector);
	base_rg->setWorldTransform(tr_base_reset);

	m_engine.m_world->addRigidBody(base_rg);
	m_engine.m_world->addRigidBody(pendule_rg);
	m_engine.m_world->addConstraint(hinge, true);

    float torque = rd_uni(rd_gen) * (rd_uni(rd_gen) > 0.5f ? -1.f : 1.f);

    pendule_rg->applyTorqueImpulse(btVector3(0.f, 0.f, torque));

    for (int i = 0; i < reset_nb_frame; i++)
        m_engine.m_world->stepSimulation(1.f / 60.f);

    episode_step = 0;
    last_action = 0.f;
    last_theta_dt = pendule_rg->getAngularVelocity().z();

    return compute_new_state();
}

bool PendulumEnv::is_action_discrete() {
	return false;
}
