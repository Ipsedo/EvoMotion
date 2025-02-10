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
    CartPole(
        int num_threads, int seed, float slider_speed, float slider_force, float chariot_push_force,
        float limit_angle, int reset_frame_nb, float chariot_mass, float pendulum_mass,
        int max_steps);

    std::vector<std::shared_ptr<AbstractItem>> get_draw_items() override;
    std::vector<std::shared_ptr<Controller>> get_controllers() override;

    std::vector<int64_t> get_state_space() override;
    std::vector<int64_t> get_action_space() override;

    std::optional<std::shared_ptr<AbstractItem>> get_camera_track_item() override;

protected:
    step compute_step() override;

    void reset_engine() override;

private:
    float chariot_push_force;

    float limit_angle;

    int reset_frame_nb;

    float chariot_pos;
    float pendulum_pos;

    std::vector<std::shared_ptr<RigidBodyItem>> items;
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