//
// Created by samuel on 10/09/19.
//

#ifndef EVOMOTION_PENDULUM_H
#define EVOMOTION_PENDULUM_H


#include <random>
#include "../environment.h"

class PendulumParams {
protected:
	float pendule_mass, hinge_force, hinge_max_speed;
	int max_step;
public:
	PendulumParams();
};

class PendulumEnv : public Environment, public PendulumParams {
public:
	torch::IntArrayRef action_space() override;

	torch::IntArrayRef state_space() override;

	explicit PendulumEnv(int seed);

protected:
	void act(torch::Tensor action) override;

	env_step compute_new_state() override;

	env_step reset_engine() override;

private:
	std::default_random_engine rd_gen;
	std::uniform_real_distribution<float> rd_uni;

	btRigidBody *pendule_rg;
	btHingeConstraint *hinge;

	std::vector<item> init_items();
};


#endif //EVOMOTION_PENDULUM_H
