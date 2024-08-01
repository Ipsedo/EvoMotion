//
// Created by samuel on 28/01/24.
//

#include "./creature_env.h"

#include "../controller/muscle_controller.h"
#include "./constants.h"

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>

MuscleEnv::MuscleEnv(const int seed)
    : rng(seed), rd_uni(0.f, 1.f),
      base(
          "base", std::make_shared<ObjShape>("./resources/obj/cube.obj"),
          glm::translate(glm::mat4(1), glm::vec3(0.f, -2.f, 2.f)), glm::vec3(1000.f, 1.f, 1000.f),
          0.f),
      skeleton_json_path("./resources/skeleton/spider_new.json"),
      skeleton(skeleton_json_path, "spider", glm::mat4(1.f)),
      muscular_system(skeleton, skeleton_json_path), initial_remaining_seconds(1.f),
      max_episode_seconds(60.f), target_velocity(1e-1f), reset_frames(10), curr_step(0),
      max_steps(static_cast<int>(max_episode_seconds / DELTA_T_MODEL)),
      remaining_steps(static_cast<int>(initial_remaining_seconds / DELTA_T_MODEL)) {
    base.get_body()->setFriction(0.5f);

    add_item(base);

    auto items = skeleton.get_items();

    auto root_item = items[0];
    add_item(root_item);
    root_item.get_body()->setActivationState(DISABLE_DEACTIVATION);

    states.push_back(std::make_shared<ItemState>(root_item, base, m_world));

    for (const auto &item: std::vector<Item>(items.begin() + 1, items.end())) {
        states.push_back(std::make_shared<ItemState>(item, root_item, base, m_world));
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

std::vector<Item> MuscleEnv::get_items() {
    auto items = skeleton.get_items();
    for (auto m: muscular_system.get_muscles())
        for (const auto &i: m.get_items()) items.push_back(i);
    items.push_back(base);
    return items;
}

std::vector<std::shared_ptr<Controller> > MuscleEnv::get_controllers() { return controllers; }

step MuscleEnv::compute_step() {
    std::vector<torch::Tensor> current_states;

    for (const auto &state: states) current_states.push_back(state->get_state().to(curr_device));

    const Item root = skeleton.get_items()[0];

    const float lin_vel_z = root.get_body()->getLinearVelocity().z();
    const float reward = lin_vel_z;

    if (lin_vel_z < target_velocity) remaining_steps -= 1;
    else remaining_steps += 1;

    const bool win = curr_step >= max_steps;
    const bool fail = remaining_steps <= 0;
    const bool done = win | fail;

    curr_step += 1;

    return {torch::cat(current_states, 0), reward, done};
}

void MuscleEnv::reset_engine() {
    // reset model transform
    glm::vec3 root_pos(1.f, 0.25f, 2.f);

    float angle_limit = static_cast<float>(M_PI) / 4.f;

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

    curr_step = 0;
    remaining_steps = static_cast<int>(initial_remaining_seconds / DELTA_T_MODEL);

    for (int i = 0; i < reset_frames; i++) m_world->stepSimulation(DELTA_T_MODEL);
}

std::vector<int64_t> MuscleEnv::get_state_space() {
    int nb_features = 0;
    for (const auto &s: states) nb_features += s->get_size();
    return {nb_features};
}

std::vector<int64_t> MuscleEnv::get_action_space() {
    return {static_cast<long>(controllers.size())};
}

bool MuscleEnv::is_continuous() const { return true; }
