//
// Created by samuel on 13/01/25.
//

#include "./robot_jump.h"

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/euler_angles.hpp>

#include "../controller/muscle_controller.h"
#include "./constants.h"

RobotJump::RobotJump(
    int num_threads, int seed, const std::string &skeleton_json_path, float minimal_velocity,
    float target_velocity, float max_seconds, float initial_seconds, float reset_seconds)
    : Environment(num_threads), rng(seed), rd_uni(0.f, 1.f), skeleton_json_path(skeleton_json_path),
      skeleton(skeleton_json_path, "robot", glm::mat4(1.f)),
      muscular_system(skeleton, skeleton_json_path),
      base(
          "base", std::make_shared<ObjShape>("./resources/obj/cube.obj"),
          glm::translate(glm::mat4(1), glm::vec3(0.f, -2.f, 2.f)), glm::vec3(1000.f, 1.f, 1000.f),
          0.f, TILE_SPECULAR),
      root_item(skeleton.get_item(skeleton.get_root_name())), controllers(), states(),
      max_steps(static_cast<int>(max_seconds / DELTA_T_MODEL)), curr_steps(0),
      initial_steps(static_cast<int>(initial_seconds / DELTA_T_MODEL)),
      remaining_steps(initial_steps), reset_frames(static_cast<int>(reset_seconds / DELTA_T_MODEL)),
      minimal_velocity(minimal_velocity), target_velocity(target_velocity) {
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

    for (auto m: muscular_system.get_muscles()) {
        for (const auto &item: m.get_items()) {
            add_item(item);
            item.get_body()->setActivationState(DISABLE_DEACTIVATION);
        }
        for (const auto c: m.get_constraints()) m_world->addConstraint(c);
        states.push_back(std::make_shared<MuscleState>(m));
    }

    for (const auto constraint: skeleton.get_constraints()) m_world->addConstraint(constraint);

    for (int i = 0; i < muscular_system.get_muscles().size(); i++)
        controllers.push_back(
            std::make_shared<MuscleController>(muscular_system.get_muscles()[i], i));
}

std::vector<Item> RobotJump::get_items() {
    auto items = skeleton.get_items();
    for (auto m: muscular_system.get_muscles())
        for (const auto &i: m.get_items()) items.push_back(i);
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

std::optional<Item> RobotJump::get_camera_track_item() { return root_item; }

step RobotJump::compute_step() {
    std::vector<torch::Tensor> current_states;

    for (const auto &state: states) current_states.push_back(state->get_state(curr_device));

    const auto velocity = std::max(root_item.get_body()->getLinearVelocity().y(), 0.f);
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

    for (auto muscle: muscular_system.get_muscles()) {
        for (const auto &item: muscle.get_items()) {
            m_world->removeRigidBody(item.get_body());
            item.reset(model_matrix);
        }
        for (auto c: muscle.get_constraints()) m_world->removeConstraint(c);
    }

    // re-add
    for (const auto &item: skeleton.get_items()) m_world->addRigidBody(item.get_body());
    for (auto m: muscular_system.get_muscles()) {
        for (const auto &item: m.get_items()) m_world->addRigidBody(item.get_body());
        for (auto c: m.get_constraints()) m_world->addConstraint(c);
    }

    for (int i = 0; i < reset_frames; i++) step_world(DELTA_T_MODEL);

    remaining_steps = initial_steps;
    curr_steps = 0;
}
