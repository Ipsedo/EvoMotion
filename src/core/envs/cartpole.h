//
// Created by samuel on 13/08/19.
//

#ifndef EVOMOTION_CARTPOLE_H
#define EVOMOTION_CARTPOLE_H

#include <random>
#include "../environment.h"

//environment create_cartpole_env();

class CartPoleEnv : public Environment {
private:
    /*
     * Bullet stuff
     */
    btHingeConstraint *hinge;
    btSliderConstraint *slider;

    btRigidBody *base_rg;
    btRigidBody *chariot_rg;
    btRigidBody *pendule_rg;

    /*
     * Random stuff
     */
    std::default_random_engine rd_gen;
    std::uniform_real_distribution<float> rd_uni;

    /*
     * Init cartpole env methods
     */
    std::vector<item> init_cartpole();

    /*
     * Env params
     */
    float slider_speed;
    float chariot_push_force;
    float chariot_pos;
    float pendule_pos;

public:
    explicit CartPoleEnv(int seed);

    torch::IntArrayRef action_space() override;

    torch::IntArrayRef state_space() override;

    ~CartPoleEnv() override;

protected:
    void act(torch::Tensor action) override;

    env_step compute_new_state() override;

    env_step reset_engine() override;
};

#endif //EVOMOTION_CARTPOLE_H
