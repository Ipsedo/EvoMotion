//
// Created by samuel on 28/01/24.
//

#include "./robot_walk.h"

#include "../controller/muscle_controller.h"
#include "../json_serializer.h"
#include "./constants.h"

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>

RobotWalk::RobotWalk(
    const int num_threads, const int seed, const std::string &skeleton_json_path,
    float initial_remaining_seconds, float max_episode_seconds, float target_velocity,
    float minimal_velocity, int reset_frames)
    : Environment(num_threads), rng(seed), rd_uni(0.f, 1.f),
      base(
          "base", std::make_shared<ObjShape>("./resources/obj/cube.obj"),
          glm::translate(glm::mat4(1), glm::vec3(0.f, -2.f, 2.f)), glm::vec3(1000.f, 1.f, 1000.f),
          0.f, TILE_SPECULAR),
      skeleton_json_path(skeleton_json_path),
      skeleton(std::make_shared<JsonDeserializer>(std::filesystem::path(skeleton_json_path))),
      initial_remaining_seconds(initial_remaining_seconds), target_velocity(target_velocity),
      minimal_velocity(minimal_velocity), reset_frames(reset_frames), curr_step(0),
      max_steps(static_cast<int>(max_episode_seconds / DELTA_T_MODEL)),
      remaining_steps(static_cast<int>(initial_remaining_seconds / DELTA_T_MODEL)),
      root_item(skeleton.get_member(skeleton.get_root_name())->get_item()) {
    base.get_body()->setFriction(0.5f);

    add_item(base);

    auto items = skeleton.get_items();
    std::vector<Item> non_root_items;
    std::copy_if(
        items.begin(), items.end(), std::back_inserter(non_root_items),
        [this](const auto &i) { return i.get_name() != skeleton.get_root_name(); });

    add_item(root_item);
    root_item.get_body()->setActivationState(DISABLE_DEACTIVATION);

    states.push_back(std::make_shared<RootMemberState>(root_item, base, m_world));

    for (const auto &item: non_root_items) {
        states.push_back(std::make_shared<MemberState>(item, root_item, base, m_world));
        add_item(item);
        item.get_body()->setActivationState(DISABLE_DEACTIVATION);
    }

    int i = 0;
    for (const auto &m: skeleton.get_muscles()) {
        states.push_back(std::make_shared<MuscleState>(m));
        controllers.push_back(std::make_shared<MuscleController>(m, i++));
    }
}

std::vector<Item> RobotWalk::get_items() {
    auto items = skeleton.get_items();
    items.push_back(base);
    return items;
}

std::vector<std::shared_ptr<Controller>> RobotWalk::get_controllers() { return controllers; }

step RobotWalk::compute_step() {
    std::vector<torch::Tensor> current_states;

    for (const auto &state: states) current_states.push_back(state->get_state(curr_device));

    const float lin_vel_z = root_item.get_body()->getLinearVelocity().z();
    const float reward = lin_vel_z;

    if (lin_vel_z < minimal_velocity) remaining_steps -= 1;
    else if (lin_vel_z >= target_velocity) remaining_steps += 1;

    const bool win = curr_step >= max_steps;
    const bool fail = remaining_steps <= 0;
    const bool done = win | fail;

    curr_step += 1;

    return {torch::cat(current_states, 0), reward, done};
}

void RobotWalk::reset_engine() {
    // reset model transform
    glm::vec3 root_pos(1.f, 0.25f, 2.f);

    float angle_limit = static_cast<float>(M_PI) / 3.f;

    float angle_yaw = rd_uni(rng) * angle_limit - angle_limit / 2.f;
    float angle_roll = rd_uni(rng) * angle_limit - angle_limit / 2.f;
    float angle_pitch = rd_uni(rng) * angle_limit - angle_limit / 2.f;
    glm::mat4 model_matrix = glm::translate(glm::mat4(1.f), root_pos)
                             * glm::eulerAngleYXZ(angle_yaw, angle_pitch, angle_roll);

    for (const auto &item: skeleton.get_items()) {
        m_world->removeRigidBody(item.get_body());
        item.reset(model_matrix);
    }
    for (const auto &c: skeleton.get_constraints()) m_world->removeConstraint(c);

    // re-add
    for (const auto &item: skeleton.get_items()) m_world->addRigidBody(item.get_body());
    for (const auto &c: skeleton.get_constraints()) m_world->addConstraint(c);

    for (int i = 0; i < reset_frames; i++) step_world(DELTA_T_MODEL);

    curr_step = 0;
    remaining_steps = static_cast<int>(initial_remaining_seconds / DELTA_T_MODEL);

    for (int i = 0; i < reset_frames; i++) step_world(DELTA_T_MODEL);
}

std::vector<int64_t> RobotWalk::get_state_space() {
    return {std::accumulate(
        states.begin(), states.end(), 0, [](auto a, const auto &s) { return a + s->get_size(); })};
}

std::vector<int64_t> RobotWalk::get_action_space() {
    return {static_cast<long>(controllers.size())};
}

std::optional<Item> RobotWalk::get_camera_track_item() { return root_item; }
