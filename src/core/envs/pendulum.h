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

    float pendule_pos_y, pendule_pos_z, pendule_height;
    float base_pos_y, base_pos_z;
public:
	PendulumParams();
};

class PendulumEnv : public PendulumParams, public Environment {
public:
	torch::IntArrayRef action_space() override;

	torch::IntArrayRef state_space() override;

	explicit PendulumEnv(long seed);

    bool is_action_discrete() override;

protected:
	void act(torch::Tensor action) override;

	env_step compute_new_state() override;

	env_step reset_engine() override;

private:
    float theta;
    // /!\ those fields are initialized in init_items method

	int episode_step;
	float last_action;

	std::mt19937 rd_gen;
	std::uniform_real_distribution<float> rd_uni;

	btRigidBody *base_rg;
	btRigidBody *pendule_rg;
	btHingeConstraint *hinge;

	std::vector<item> init_items();
};


#endif //EVOMOTION_PENDULUM_H
