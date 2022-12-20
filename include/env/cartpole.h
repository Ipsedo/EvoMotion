//
// Created by samuel on 18/12/22.
//

#ifndef EVO_MOTION_CARTPOLE_H
#define EVO_MOTION_CARTPOLE_H

#include <random>
#include <btBulletDynamicsCommon.h>

#include "./model/environment.h"

class SliderController : public Controller {
public:
    explicit SliderController(btSliderConstraint *slider, float slider_speed);

    void on_input(torch::Tensor action) override;

private:
    btSliderConstraint *slider;
    float slider_speed;
};

class CartPole : public Environment {
public:
    explicit CartPole(int seed);

    std::vector<Item> get_items() override;

    std::vector<std::shared_ptr<Controller>> get_controllers() override;

protected:
    step compute_step() override;

    void reset_engine() override;

private:
    float slider_speed;
    float slider_force;

    float chariot_push_force;

    float limit_angle;

    int reset_frame_nb;

    float chariot_mass;
    float pendulum_mass;

    float chariot_pos;
    float pendulum_pos;

    std::vector<Item> items;
    std::vector<std::shared_ptr<Controller>> controllers;

    btHingeConstraint *hinge;
    btSliderConstraint *slider;

    btRigidBody *base_rg;
    btRigidBody *chariot_rg;
    btRigidBody *pendulum_rg;

    std::mt19937 rng;
    std::uniform_real_distribution<float> rd_uni;

    int step_idx;
    int max_steps;

    float last_vel;
    float last_ang_vel;

};

#endif //EVO_MOTION_CARTPOLE_H
