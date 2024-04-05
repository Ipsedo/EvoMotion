//
// Created by samuel on 28/01/24.
//

#include "./env_test_muscle.h"
#include "../controller/muscle_controller.h"
#include "../model/skeleton.h"
#include "../model/muscle.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

MuscleEnv::MuscleEnv(int seed) :
    Environment(),
    rng(seed),
    rd_uni(0.f, 1.f),
    items(),
    constraints(),
    controllers(),
    states(),
    reset_angle_torque(M_PI / 2.f),
    reset_frames(8),
    reset_torque_force(10.f),
    curr_step(0),
    max_steps(60 * 10) {

    Item base("base", std::make_shared<ObjShape>("./resources/obj/cube.obj"),
              glm::translate(glm::mat4(1), glm::vec3(0.f, -2.f, 2.f)),
              glm::vec3(1000.f, 1.f, 1000.f), 0.f);

    auto json_path = "./resources/skeleton/spider_4.json";
    JsonSkeleton json_skeleton(
        json_path,
        "skeleton_test",
        glm::translate(glm::mat4(1.0), glm::vec3(1.f, 0.f, 2.f)));

    JsonMuscularSystem json_muscular_system(
        json_skeleton,
        json_path);

    for (const Item &item: json_skeleton.get_items()) {
        items.push_back(item);
        states.emplace_back(item);
    }

    for (auto m: json_muscular_system.get_muscles()) {
        for (const auto &item: m.get_items()) {
            items.push_back(item);
            states.emplace_back(item);
        }
        for (auto c: m.get_constraints())
            constraints.push_back(c);
    }

    items.push_back(base);

    for (auto item: items) {
        item.get_body()->setActivationState(DISABLE_DEACTIVATION);
        add_item(item);
    }

    for (auto c: constraints)
        m_world->addConstraint(c);

    for (auto constraint: json_skeleton.get_constraints())
        m_world->addConstraint(constraint);

    //controllers.push_back(std::make_shared<MuscleController>(json_muscular_system.get_muscles()[0], 0));
    for (int i = 0; i < json_muscular_system.get_muscles().size(); i++)
        controllers.push_back(std::make_shared<MuscleController>(json_muscular_system.get_muscles()[i], i));
}

std::vector<Item> MuscleEnv::get_items() {
    return items;
}

std::vector<std::shared_ptr<Controller>> MuscleEnv::get_controllers() {
    return controllers;
}

step MuscleEnv::compute_step() {
    std::vector<torch::Tensor> current_states;

    for (auto state: states)
        current_states.push_back(state.get_state().to(curr_device));

    float reward = items[0].get_body()->getCenterOfMassPosition().z();

    curr_step += 1;

    return {
        torch::cat(current_states, 0), reward, curr_step >= max_steps
    };
}

void MuscleEnv::reset_engine() {
    for (auto item: items) {
        m_world->removeRigidBody(item.get_body());
        item.reset();
    }

    for (auto c: constraints)
        m_world->removeConstraint(c);

    for (auto item: items)
        m_world->addRigidBody(item.get_body());
    for (auto c: constraints)
        m_world->addConstraint(c);

    // prevent overfitting ?
    Item root = items[0]; // TODO better way to get root item ?

    glm::vec3 axis = glm::rotate(glm::mat4(1), rd_uni(rng) * reset_angle_torque, glm::vec3(1, 0, 0)) *
                     glm::rotate(glm::mat4(1), float(rd_uni(rng) * M_PI) * 2.f, glm::vec3(0, 1, 0)) *
                     glm::vec4(glm::vec3(0, 1, 0), 0);
    /*glm::vec3 point(0);

    auto perpendicular_component = glm::cross(axis, glm::cross(axis, point));
    auto torque_direction = glm::normalize(glm::cross(point, perpendicular_component));
    auto torque_vector = reset_torque_force * torque_direction;*/

    root.get_body()->applyTorqueImpulse(glm_to_bullet(axis * reset_torque_force));

    for (int i = 0; i < reset_frames; i++)
        m_world->stepSimulation(1.f / 60.f);

    curr_step = 0;
}

std::vector<int64_t> MuscleEnv::get_state_space() {
    int nb_features = 0;
    for (auto s: states)
        nb_features += s.get_size();
    return {nb_features};
}

std::vector<int64_t> MuscleEnv::get_action_space() {
    return {(long) controllers.size()};
}

bool MuscleEnv::is_continuous() const {
    return true;
}
