//
// Created by samuel on 18/12/22.
//

#ifndef EVO_MOTION_CARTPOLE_H
#define EVO_MOTION_CARTPOLE_H

#include <random>

#include <btBulletDynamicsCommon.h>

#include <evo_motion_model/environment.h>

class CartPole final : public Environment {
public:
    explicit CartPole(int seed);

    std::vector<Item> get_items() override;

    std::vector<std::shared_ptr<Controller>> get_controllers() override;

    std::vector<int64_t> get_state_space() override;

    std::vector<int64_t> get_action_space() override;

    [[nodiscard]] bool is_continuous() const override;

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

#endif//EVO_MOTION_CARTPOLE_H