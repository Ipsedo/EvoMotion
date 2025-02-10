//
// Created by samuel on 13/01/25.
//

#include "./robot_jump.h"

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/euler_angles.hpp>

#include "../controller/muscle_controller.h"
#include "../json_serializer.h"
#include "./constants.h"

RobotJump::RobotJump(
    const int num_threads, const int seed, const std::string &skeleton_json_path,
    const float minimal_velocity, const float target_velocity, const float max_seconds,
    const float initial_seconds, const float reset_seconds)
    : Environment(num_threads), rng(seed), rd_uni(0.f, 1.f), skeleton_json_path(skeleton_json_path),
      skeleton(std::make_shared<JsonDeserializer>(std::filesystem::path(skeleton_json_path))),
      base(std::make_shared<Item>(
          "base", std::make_shared<ObjShape>("./resources/obj/cube.obj"),
          glm::translate(glm::mat4(1), glm::vec3(0.f, -2.f, 2.f)), glm::vec3(1000.f, 1.f, 1000.f),
          0.f, TILE_SPECULAR)),
      root_item(skeleton.get_member(skeleton.get_root_name())->get_item()), controllers(), states(),
      max_steps(static_cast<int>(max_seconds / DELTA_T_MODEL)), curr_steps(0),
      initial_steps(static_cast<int>(initial_seconds / DELTA_T_MODEL)),
      remaining_steps(initial_steps), reset_frames(static_cast<int>(reset_seconds / DELTA_T_MODEL)),
      minimal_velocity(minimal_velocity), target_velocity(target_velocity) {
    base->get_body()->setFriction(0.5f);

    m_world->addRigidBody(base->get_body());

    for (const auto &b: skeleton.get_bodies()) {
        m_world->addRigidBody(b);
        b->setActivationState(DISABLE_DEACTIVATION);
    }

    for (const auto &c: skeleton.get_constraints()) m_world->addConstraint(c);

    states = skeleton.get_states(base, m_world);
    controllers = skeleton.get_controllers();
}

std::vector<std::shared_ptr<AbstractItem>> RobotJump::get_draw_items() {
    auto items = skeleton.get_items();
    items.push_back(base);
    return items;
}

std::vector<std::shared_ptr<Controller>> RobotJump::get_controllers() { return controllers; }

std::vector<int64_t> RobotJump::get_state_space() {
    return {std::accumulate(
        states.begin(), states.end(), 0, [](auto a, const auto &s) { return a + s->get_size(); })};
}

std::vector<int64_t> RobotJump::get_action_space() {
    return {static_cast<long>(controllers.size())};
}

std::optional<std::shared_ptr<AbstractItem>> RobotJump::get_camera_track_item() {
    return root_item;
}

step RobotJump::compute_step() {
    std::vector<torch::Tensor> current_states;

    for (const auto &state: states) current_states.push_back(state->get_state(curr_device));

    const auto velocity = std::max(root_item->get_body()->getLinearVelocity().y(), 0.f)
                          + root_item->get_body()->getLinearVelocity().z();
    const float reward = velocity;

    if (velocity < minimal_velocity) remaining_steps -= 1;
    else if (velocity >= target_velocity) remaining_steps += 1;

    const auto win = curr_steps >= max_steps;
    const auto fail = remaining_steps < 0;
    const bool done = win | fail;

    curr_steps += 1;

    return {torch::cat(current_states, 0), reward, done};
}

void RobotJump::reset_engine() {
    constexpr glm::vec3 root_pos(1.f, 0.25f, 2.f);

    constexpr float angle_limit = static_cast<float>(M_PI) / 3.f;

    const float angle_yaw = rd_uni(rng) * angle_limit - angle_limit / 2.f;
    const float angle_roll = rd_uni(rng) * angle_limit - angle_limit / 2.f;
    const float angle_pitch = rd_uni(rng) * angle_limit - angle_limit / 2.f;
    const glm::mat4 model_matrix = glm::translate(glm::mat4(1.f), root_pos)
                                   * glm::eulerAngleYXZ(angle_yaw, angle_pitch, angle_roll);

    for (const auto &b: skeleton.get_bodies()) m_world->removeRigidBody(b);
    for (const auto &i: skeleton.get_items()) i->reset(model_matrix);
    for (const auto &c: skeleton.get_constraints()) m_world->removeConstraint(c);

    // re-add
    for (const auto &b: skeleton.get_bodies()) m_world->addRigidBody(b);
    for (const auto &c: skeleton.get_constraints()) m_world->addConstraint(c);

    for (int i = 0; i < reset_frames; i++) step_world(DELTA_T_MODEL);

    remaining_steps = initial_steps;
    curr_steps = 0;
}
