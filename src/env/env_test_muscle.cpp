//
// Created by samuel on 28/01/24.
//

#include "./env_test_muscle.h"
#include "../controller/muscle_controller.h"
#include "../model/skeleton.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

MuscleEnv::MuscleEnv() : Environment({1}, {1}, true),
                         items(),
                         controllers() {

    Item base("base", std::make_shared<ObjShape>("./resources/obj/cube.obj"),
              glm::translate(glm::mat4(1), glm::vec3(0.f, -5.f, 5.f)),
              glm::vec3(10.f, 1.f, 10.f), 0.f);

    float fixed_angle = M_PI - M_PI / 4.f;
    glm::vec3 base_rot_point(0, -1, 0);
    glm::vec3 base_member_pos(0, 2, 0);

    glm::mat4 translation_to_origin = glm::translate(glm::mat4(1.0f),
                                                     -base_rot_point);
    glm::mat4 rotation_matrix = glm::rotate(glm::mat4(1.0f), fixed_angle,
                                            glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 translation_back = glm::translate(glm::mat4(1.0f),
                                                base_rot_point);
    glm::mat4 translation_to_position = glm::translate(glm::mat4(1.0f),
                                                       base_member_pos);

    auto [member_base, fixed_constraint] = base.attach_item_fixed(
        translation_to_position * translation_back * rotation_matrix *
        translation_to_origin,
        glm::translate(glm::mat4(1.), glm::vec3(0, 1, 0)) * rotation_matrix,
        glm::translate(glm::mat4(1.), glm::vec3(0, -1, 0)),
        "member_base",
        std::make_shared<ObjShape>(
            "./resources/obj/cube.obj"),
        glm::vec3(0.1f, 1.f, 0.1f), 1.f);

    float hinge_angle = -float(M_PI) / 2.;
    glm::vec3 member_rot_point(0, -1, 0);
    glm::vec3 member_pos(0, 2, 0);

    translation_to_origin = glm::translate(glm::mat4(1.0f), -member_rot_point);
    rotation_matrix = glm::rotate(glm::mat4(1.0f), hinge_angle,
                                  glm::vec3(0.0f, 0.0f, 1.0f));
    translation_back = glm::translate(glm::mat4(1.0f), member_rot_point);
    translation_to_position = glm::translate(glm::mat4(1.0f), member_pos);

    auto [member, hinge] = member_base.attach_item_hinge(
        translation_to_position * translation_back * rotation_matrix *
        translation_to_origin,
        glm::translate(glm::mat4(1.), glm::vec3(0, 1, 0)),
        glm::translate(glm::mat4(1.), glm::vec3(0, -1, 0)),
        glm::vec3(0, 0, 1),
        "member",
        std::make_shared<ObjShape>("./resources/obj/cube.obj"),
        glm::vec3(0.1f, 1.f, 0.1f), 1.f
    );

    muscle = std::make_shared<Muscle>("test_muscle", 0.01f, glm::vec3(0.1f),
                                      member_base, glm::vec3(0.1, 0.2, 0.0),
                                      member,
                                      glm::vec3(0.1, 0.2, 0));


    JsonSkeleton json_skeleton(
        "./resources/skeleton/test_1.json",
        "skeleton_test",
        glm::translate(glm::mat4(1.0), glm::vec3(1.f, -1.f, 5.f))
    );


    items = {base, member_base, member};
    for (const auto &item: muscle->get_items())
        items.push_back(item);
    for (const Item &item: json_skeleton.get_items())
        items.push_back(item);

    for (auto item: items) {
        item.get_body()->setActivationState(DISABLE_DEACTIVATION);
        add_item(item);
    }

    for (auto constraint: muscle->get_constraints())
        m_world->addConstraint(constraint);

    for (auto constraint: json_skeleton.get_constraints())
        m_world->addConstraint(constraint);

    m_world->addConstraint(fixed_constraint);
    m_world->addConstraint(hinge);

    controllers.push_back(std::make_shared<MuscleController>(*muscle, 0));

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

}
