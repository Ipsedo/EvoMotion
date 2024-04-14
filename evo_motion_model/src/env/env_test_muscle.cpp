//
// Created by samuel on 28/01/24.
//

#include "./env_test_muscle.h"

#include <chrono>

#include "../controller/muscle_controller.h"

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>

int64_t get_time_millis() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::system_clock::now().time_since_epoch())
        .count();
}

MuscleEnv::MuscleEnv(int seed)
    : Environment(), rng(get_time_millis()), rd_uni(0.f, 1.f),
      base(
          "base", std::make_shared<ObjShape>("./resources/obj/cube.obj"),
          glm::translate(glm::mat4(1), glm::vec3(0.f, -2.f, 2.f)), glm::vec3(1000.f, 1.f, 1000.f),
          0.f),
      skeleton_json_path("./resources/skeleton/spider_new.json"),
      skeleton(skeleton_json_path, "spider", glm::mat4(1.f)),
      muscular_system(skeleton, skeleton_json_path), controllers(), states(), curr_step(0),
      max_steps(60 * 60), nb_steps_without_moving(0), max_steps_without_moving(60 * 2),
      velocity_delta(0.2) {

    base.get_body()->setFriction(100.f);

    add_item(base);

    for (Item item: skeleton.get_items()) {
        states.emplace_back(item);
        add_item(item);
        item.get_body()->setActivationState(DISABLE_DEACTIVATION);
    }

    for (auto m: muscular_system.get_muscles()) {
        for (auto item: m.get_items()) {
            add_item(item);
            item.get_body()->setActivationState(DISABLE_DEACTIVATION);
        }
        for (auto c: m.get_constraints()) m_world->addConstraint(c);
    }

    for (auto constraint: skeleton.get_constraints()) m_world->addConstraint(constraint);

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

std::vector<std::shared_ptr<Controller>> MuscleEnv::get_controllers() { return controllers; }

step MuscleEnv::compute_step() {
    std::vector<torch::Tensor> current_states;

    for (auto state: states) current_states.push_back(state.get_state().to(curr_device));

    curr_step += 1;

    Item root = skeleton.get_items()[0];

    if (root.get_body()->getLinearVelocity().z() <= velocity_delta) nb_steps_without_moving += 1;
    else nb_steps_without_moving = 0;

    bool win = curr_step >= max_steps;
    bool fail = nb_steps_without_moving >= max_steps_without_moving;

    float reward = root.get_body()->getLinearVelocity().z() - velocity_delta +
                   float(curr_step) / float(max_steps);

    bool done = win | fail;

    return {torch::cat(current_states, 0), reward, done};
}

void MuscleEnv::reset_engine() {
    rng = std::mt19937(get_time_millis());

    // reset model transform
    glm::vec3 root_pos(1.f, 0.f, 2.f);

    float angle_limit = float(M_PI) / 6.f;

    float angle_yaw = rd_uni(rng) * angle_limit - angle_limit / 2.f;
    float angle_roll = rd_uni(rng) * angle_limit - angle_limit / 2.f;
    float angle_pitch = rd_uni(rng) * angle_limit - angle_limit / 2.f;
    glm::mat4 model_matrix = glm::translate(glm::mat4(1.f), root_pos) *
                             glm::eulerAngleYXZ(angle_yaw, angle_pitch, angle_roll);

    for (auto item: skeleton.get_items()) {
        m_world->removeRigidBody(item.get_body());
        item.reset(model_matrix);
    }

    for (auto muscle: muscular_system.get_muscles()) {
        for (auto item: muscle.get_items()) {
            m_world->removeRigidBody(item.get_body());
            item.reset(model_matrix);
        }
        for (auto c: muscle.get_constraints()) m_world->removeConstraint(c);
    }

    // re-add
    for (auto item: skeleton.get_items()) m_world->addRigidBody(item.get_body());
    for (auto m: muscular_system.get_muscles()) {
        for (auto item: m.get_items()) m_world->addRigidBody(item.get_body());
        for (auto c: m.get_constraints()) m_world->addConstraint(c);
    }

    curr_step = 0;
    nb_steps_without_moving = 0;
}

std::vector<int64_t> MuscleEnv::get_state_space() {
    int nb_features = 0;
    for (auto s: states) nb_features += s.get_size();
    return {nb_features};
}

std::vector<int64_t> MuscleEnv::get_action_space() { return {(long) controllers.size()}; }

bool MuscleEnv::is_continuous() const { return true; }
