//
// Created by samuel on 13/01/25.
//

#ifndef EVO_MOTION_ROBOT_JUMP_H
#define EVO_MOTION_ROBOT_JUMP_H

#include <random>

#include <evo_motion_model/environment.h>

#include "../creature/muscle.h"
#include "../creature/skeleton.h"
#include "../creature/state.h"

class RobotJump final : public Environment {
public:
    RobotJump(
        int num_threads, int seed, const std::string &skeleton_json_path, float minimal_height,
        float target_height, float max_seconds, float initial_seconds, float reset_seconds);

    std::vector<Item> get_items() override;
    std::vector<std::shared_ptr<Controller>> get_controllers() override;
    std::vector<int64_t> get_state_space() override;
    std::vector<int64_t> get_action_space() override;
    std::optional<Item> get_camera_track_item() override;

protected:
    step compute_step() override;
    void reset_engine() override;

private:
    std::mt19937 rng;
    std::uniform_real_distribution<float> rd_uni;

    std::string skeleton_json_path;
    JsonSkeleton skeleton;
    JsonMuscularSystem muscular_system;

    Item base;
    Item root_item;

    std::vector<std::shared_ptr<Controller>> controllers;

    std::vector<std::shared_ptr<State>> states;

    int max_steps;
    int curr_steps;
    int remaining_steps;
    int initial_steps;
    int reset_frames;

    float initial_height;
    float minimal_height;
    float target_height;
};

#endif//EVO_MOTION_ROBOT_JUMP_H
