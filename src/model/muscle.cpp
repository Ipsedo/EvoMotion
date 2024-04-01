//
// Created by samuel on 30/12/23.
//

#include <memory>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtx/euler_angles.hpp>
#include <iostream>

#include "./muscle.h"


glm::mat4 get_rotation(glm::vec3 a, glm::vec3 b) {
    return glm::rotate(glm::mat4(1.0f), acos(
                           glm::dot(b, a) / (glm::length(b) * glm::length(a))),
                       glm::cross(b, a));
}


Muscle::Muscle(
    const std::string &name,
    float attach_mass,
    glm::vec3 attach_scale,
    Item &item_a,
    glm::vec3 pos_in_a,
    Item &item_b,
    glm::vec3 pos_in_b
) {

    {
        glm::mat4 attach_a_mat = item_a.model_matrix_without_scale() * glm::translate(glm::mat4(1), pos_in_a);
        glm::mat4 attach_b_mat = item_b.model_matrix_without_scale() * glm::translate(glm::mat4(1), pos_in_b);

        attach_a = std::make_shared<Item>(name + "_attach_a", std::make_shared<ObjShape>("./resources/obj/sphere.obj"),
                                          attach_a_mat, attach_scale, attach_mass);
        attach_b = std::make_shared<Item>(name + "_attach_b", std::make_shared<ObjShape>("./resources/obj/sphere.obj"),
                                          attach_b_mat, attach_scale, attach_mass);
    }


    {
        btTransform frame_in_attach_a;
        frame_in_attach_a.setIdentity();
        btTransform frame_in_attach_b;
        frame_in_attach_b.setIdentity();

        muscle_slider_constraint = new btSliderConstraint(
            *attach_a->get_body(), *attach_b->get_body(), frame_in_attach_a,
            frame_in_attach_b,
            true);

        muscle_slider_constraint->setMaxLinMotorForce(100.f);
        muscle_slider_constraint->setTargetLinMotorVelocity(0.f);
        muscle_slider_constraint->setLowerAngLimit(0);
        muscle_slider_constraint->setUpperAngLimit(0);
    }

    {

        btTransform frame_in_item_a;
        frame_in_item_a.setOrigin(btVector3(pos_in_a.x, pos_in_a.y, pos_in_a.z));
        btTransform frame_in_attach_a;
        frame_in_attach_a.setIdentity();

        float limit_angle_cone = 30.f;

        attach_a_constraint = new btConeTwistConstraint(
            *item_a.get_body(),
            *attach_a->get_body(),
            frame_in_item_a,
            frame_in_attach_a
        );
        attach_a_constraint->setLimit(limit_angle_cone, limit_angle_cone, 0.f);

        btTransform frame_in_item_b;
        frame_in_item_b.setOrigin(btVector3(pos_in_b.x, pos_in_b.y, pos_in_b.z));
        btTransform frame_in_attach_b;
        frame_in_attach_b.setIdentity();

        attach_b_constraint = new btConeTwistConstraint(
            *item_b.get_body(),
            *attach_b->get_body(),
            frame_in_item_b,
            frame_in_attach_b
        );
        attach_b_constraint->setLimit(limit_angle_cone, limit_angle_cone, 0.f);
    }

    item_a.get_body()->setIgnoreCollisionCheck(attach_a->get_body(), true);
    item_b.get_body()->setIgnoreCollisionCheck(attach_a->get_body(), true);

    item_b.get_body()->setIgnoreCollisionCheck(attach_b->get_body(), true);
    item_b.get_body()->setIgnoreCollisionCheck(attach_a->get_body(), true);

    item_a.get_body()->setIgnoreCollisionCheck(item_b.get_body(), true);
}

void Muscle::contract(float force) {
    muscle_slider_constraint->setPoweredLinMotor(true);
    muscle_slider_constraint->setTargetLinMotorVelocity(force * 3);
}

void Muscle::release() {
    muscle_slider_constraint->setPoweredLinMotor(false);
}

std::vector<Item> Muscle::get_items() {
    return {*attach_a, *attach_b};
}

std::vector<btTypedConstraint *> Muscle::get_constraints() {
    return {muscle_slider_constraint, attach_a_constraint, attach_b_constraint};
}
