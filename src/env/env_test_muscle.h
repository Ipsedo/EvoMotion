//
// Created by samuel on 28/01/24.
//

#ifndef EVO_MOTION_ENV_TEST_MUSCLE_H
#define EVO_MOTION_ENV_TEST_MUSCLE_H

#include <random>
#include "../model/environment.h"
#include "../model/muscle.h"

#include "../model/state.h"

class MuscleEnv : public Environment {
public:
    MuscleEnv(int seed);

    std::vector<Item> get_items() override;

    std::vector<std::shared_ptr<Controller>> get_controllers() override;

    std::vector<int64_t> get_state_space() override;

    std::vector<int64_t> get_action_space() override;

    bool is_continuous() const override;

protected:
    step compute_step() override;

    void reset_engine() override;

private:
    std::mt19937 rng;
    std::uniform_real_distribution<float> rd_uni;

    std::vector<Item> items;
    std::vector<btTypedConstraint *> constraints;

    std::vector<std::shared_ptr<Controller>> controllers;

    std::vector<ItemState> states;

    float reset_angle_torque;
    int reset_frames;
    float reset_torque_force;

    int curr_step;
    int max_steps;

    int nb_steps_without_moving;
    float velocity_delta;
    int max_steps_without_moving;

};

#endif //EVO_MOTION_ENV_TEST_MUSCLE_H
