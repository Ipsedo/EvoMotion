//
// Created by samuel on 28/01/24.
//

#ifndef EVO_MOTION_ENV_TEST_MUSCLE_H
#define EVO_MOTION_ENV_TEST_MUSCLE_H

#include <random>

#include <evo_motion_model/environment.h>
#include <evo_motion_model/robot/skeleton.h>
#include <evo_motion_model/state.h>

class RobotWalk final : public Environment {
public:
    RobotWalk(
        int num_threads, int seed, const std::string &skeleton_json_path,
        float initial_remaining_seconds, float max_episode_seconds, float target_velocity,
        float minimal_velocity, int reset_frames);

    std::vector<std::shared_ptr<AbstractItem>> get_draw_items() override;
    std::vector<std::shared_ptr<Controller>> get_controllers() override;

    std::vector<int64_t> get_state_space() override;
    std::vector<int64_t> get_action_space() override;

    std::optional<std::shared_ptr<AbstractItem>> get_camera_track_item() override;

protected:
    step compute_step() override;
    void reset_engine() override;

private:
    std::mt19937 rng;
    std::uniform_real_distribution<float> rd_uni;

    std::shared_ptr<Item> base;

    std::string skeleton_json_path;
    Skeleton skeleton;

    std::vector<std::shared_ptr<Controller>> controllers;

    std::vector<std::shared_ptr<State>> states;

    float initial_remaining_seconds;

    float target_velocity;
    float minimal_velocity;

    int reset_frames;
    int curr_step;
    int max_steps;

    int remaining_steps;

    std::shared_ptr<Item> root_item;
};

#endif//EVO_MOTION_ENV_TEST_MUSCLE_H