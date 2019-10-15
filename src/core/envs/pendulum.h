//
// Created by samuel on 10/09/19.
//

#ifndef EVOMOTION_PENDULUM_H
#define EVOMOTION_PENDULUM_H


#include <random>
#include "../environment.h"

class PendulumParams {
protected:
	float pendule_mass, hinge_push_force;
	int max_step, reset_nb_frame;
public:
	PendulumParams();
};

class PendulumEnv : public PendulumParams, public Environment {
public:
	torch::IntArrayRef action_space() override;

	torch::IntArrayRef state_space() override;

	explicit PendulumEnv(int seed);

protected:
	void act(torch::Tensor action) override;

	env_step compute_new_state() override;

	env_step reset_engine() override;

private:
    // /!\ those fields are initialized in init_items method
    float pendule_pos_y, pendule_pos_z, pendule_height;

	int episode_step;
	float last_action;

	std::default_random_engine rd_gen;
	std::uniform_real_distribution<float> rd_uni;

	btRigidBody *pendule_rg;
	btHingeConstraint *hinge;

	std::vector<item> init_items();
};


#endif //EVOMOTION_PENDULUM_H
