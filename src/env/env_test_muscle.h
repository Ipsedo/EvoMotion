//
// Created by samuel on 28/01/24.
//

#ifndef EVO_MOTION_ENV_TEST_MUSCLE_H
#define EVO_MOTION_ENV_TEST_MUSCLE_H

#include <random>
#include "../model/environment.h"
#include "../model/muscle.h"
#include "../model/skeleton.h"

#include "../model/state.h"

class MuscleEnv : public Environment {
public:
    explicit MuscleEnv(int seed);

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

    Item base;

    std::string skeleton_json_path;
    JsonSkeleton skeleton;
    JsonMuscularSystem muscular_system;

    std::vector<std::shared_ptr<Controller>> controllers;

    std::vector<ItemState> states;

    int curr_step;
    int max_steps;

    int nb_steps_without_moving;
    float velocity_delta;
    int max_steps_without_moving;

};

#endif //EVO_MOTION_ENV_TEST_MUSCLE_H
