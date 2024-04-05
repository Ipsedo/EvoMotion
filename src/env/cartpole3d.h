//
// Created by samuel on 25/12/22.
//

#ifndef EVO_MOTION_CARTPOLE3D_H
#define EVO_MOTION_CARTPOLE3D_H

#include <random>

#include "../model/environment.h"

class CartPole3d : public Environment {
public:
    explicit CartPole3d(int seed);

    std::vector<Item> get_items() override;

    std::vector<std::shared_ptr<Controller>> get_controllers() override;

    std::vector<int64_t> get_state_space() override;

    std::vector<int64_t> get_action_space() override;

    bool is_continuous() const override;

protected:
    step compute_step() override;

    void reset_engine() override;

private:
    int reset_frame_nb;

    float slider_force_per_kg;
    float slider_speed;

    float chariot_push_force;

    btVector3 cart_x_scale;
    btVector3 cart_z_scale;
    btVector3 pole_scale;
    btVector3 base_scale;

    btVector3 base_pos;
    btVector3 cart_x_pos;
    btVector3 cart_z_pos;
    btVector3 pole_pos;

    float cart_x_mass;
    float cart_z_mass;
    float pole_mass;
    float base_mass;

    btRigidBody *cart_x_rg;
    btRigidBody *cart_z_rg;
    btRigidBody *pole_rg;
    btRigidBody *base_rg;

    btSliderConstraint *slider_x;
    btSliderConstraint *slider_z;

    btPoint2PointConstraint *p2p_constraint;

    std::vector<Item> items;
    std::vector<std::shared_ptr<Controller>> controllers;

    std::mt19937 rng;
    std::uniform_real_distribution<float> rd_uni;

    float last_vel_x;
    float last_vel_z;

    float last_ang;
    float last_ang_vel;
    btVector3 last_ang_vel_vec;

    float last_vert_ang;
    float last_vert_ang_vel;

    float last_plan_ang;
    float last_plan_ang_vec;

    float limit_angle;

    int step_idx;
    int max_steps;
};

#endif //EVO_MOTION_CARTPOLE3D_H
