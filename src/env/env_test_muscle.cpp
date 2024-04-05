//
// Created by samuel on 28/01/24.
//

#include "./env_test_muscle.h"
#include "../controller/muscle_controller.h"
#include "../model/skeleton.h"
#include "../model/muscle.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

MuscleEnv::MuscleEnv() : Environment({1}, {1}, true),
                         items(),
                         constraints(),
                         controllers() {

    Item base("base", std::make_shared<ObjShape>("./resources/obj/cube.obj"),
              glm::translate(glm::mat4(1), glm::vec3(0.f, -2.f, 2.f)),
              glm::vec3(10.f, 1.f, 10.f), 0.f);

    auto json_path = "./resources/skeleton/spider_4.json";
    JsonSkeleton json_skeleton(
        json_path,
        "skeleton_test",
        glm::translate(glm::mat4(1.0), glm::vec3(1.f, 0.f, 2.f)));

    JsonMuscularSystem json_muscular_system(
        json_skeleton,
        json_path);


    items = {base};
    for (const Item &item: json_skeleton.get_items())
        items.push_back(item);

    for (auto m: json_muscular_system.get_muscles()) {
        for (const auto &item: m.get_items())
            items.push_back(item);
        for (auto c: m.get_constraints())
            constraints.push_back(c);
    }

    for (auto item: items) {
        item.get_body()->setActivationState(DISABLE_DEACTIVATION);
        add_item(item);
    }

    for (auto c: constraints)
        m_world->addConstraint(c);

    for (auto constraint: json_skeleton.get_constraints())
        m_world->addConstraint(constraint);

    //controllers.push_back(std::make_shared<MuscleController>(json_muscular_system.get_muscles()[0], 0));

}

std::vector<Item> MuscleEnv::get_items() {
    return items;
}

std::vector<std::shared_ptr<Controller>> MuscleEnv::get_controllers() {
    return controllers;
}

step MuscleEnv::compute_step() {
    Item member = items[2];


    return {
        torch::zeros({1}), 1, false
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
}
