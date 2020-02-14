//
// Created by samuel on 13/08/19.
//

#ifndef EVOMOTION_CARTPOLE_H
#define EVOMOTION_CARTPOLE_H

#include <random>
#include "../environment.h"

class CartPoleEnvParams {
protected:
	/*
	 * Env params
	 */
	float slider_speed;
	float slider_force;

	float chariot_push_force;

	float limit_angle;

	int reset_frame_nb;

	float chariot_mass;
	float pendule_mass;

protected:
public:
    CartPoleEnvParams(float slider_speed, float slider_force, float chariot_push_force,
            float limit_angle, int reset_frame_nb, float chariot_mass, float pendule_mass);
};

class CartPoleEnv : public CartPoleEnvParams, public Environment {
private:

	/*
	 * Random stuff
	 */

	std::default_random_engine rd_gen;
	std::uniform_real_distribution<float> rd_uni;

	/*
	 * Init cartpole env methods
	 */

	std::vector<item> init_cartpole();

public:

	explicit CartPoleEnv(int seed, float slider_speed, float slider_force, float chariot_push_force,
                         float limit_angle, int reset_frame_nb,
                         float chariot_mass, float pendule_mass);

	torch::IntArrayRef state_space() override;

	~CartPoleEnv() override;

protected:

    /*
     * Env params
     */

    float chariot_pos;
    float pendule_pos;

	/*
	 * Bullet stuff
	 */

	btRigidBody *base_rg;
	btRigidBody *chariot_rg;
	btRigidBody *pendule_rg;

	btHingeConstraint *hinge;
	btSliderConstraint *slider;

	/*
	 * Environment stuff
	 */

	env_step compute_new_state() override;

	env_step reset_engine() override;
};

class ContinuousCartPoleEnv : public CartPoleEnv {
public:
	torch::IntArrayRef action_space() override;
	explicit ContinuousCartPoleEnv(int seed);

	bool is_action_discrete() override;

protected:
	void act(torch::Tensor action) override;
};

class DiscreteCartPoleEnv : public CartPoleEnv {
public:
    torch::IntArrayRef action_space() override;
    explicit DiscreteCartPoleEnv(int seed);

	bool is_action_discrete() override;

protected:
    void act(torch::Tensor action) override;
};

#endif //EVOMOTION_CARTPOLE_H
