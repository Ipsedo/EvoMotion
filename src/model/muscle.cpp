//
// Created by samuel on 30/12/23.
//

#include <memory>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtx/euler_angles.hpp>

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

    glm::mat4 translate_in_a = glm::translate(glm::mat4(1.), pos_in_a);
    glm::mat4 translate_in_b = glm::translate(glm::mat4(1.), pos_in_b);

    glm::vec3 a_to_b = item_b.get_last_model_matrix() * translate_in_b * glm::vec4(glm::vec3(0), 1) -
                       item_a.get_last_model_matrix() * translate_in_a * glm::vec4(glm::vec3(0), 1);
    glm::vec3 b_to_a = item_a.get_last_model_matrix() * translate_in_a * glm::vec4(glm::vec3(0), 1) -
                       item_b.get_last_model_matrix() * translate_in_b * glm::vec4(glm::vec3(0), 1);

    glm::mat4 rot_attach_a = glm::eulerAngleYXZ(a_to_b.y, a_to_b.x, a_to_b.z);
    glm::mat4 rot_attach_b = glm::eulerAngleYXZ(b_to_a.y, b_to_a.x, b_to_a.z);

    glm::mat4 attach_a_mat = item_a.get_last_model_matrix() * translate_in_a * rot_attach_a;
    glm::mat4 attach_b_mat = item_b.get_last_model_matrix() * translate_in_b * rot_attach_b;

    attach_a = std::make_shared<Item>(name + "_attach_a", std::make_shared<ObjShape>("./resources/obj/cube.obj"),
                                      attach_a_mat, attach_scale, attach_mass);
    attach_b = std::make_shared<Item>(name + "_attach_b", std::make_shared<ObjShape>("./resources/obj/cube.obj"),
                                      attach_b_mat, attach_scale, attach_mass);

    btTransform frame_in_a_tr;
    frame_in_a_tr.setFromOpenGLMatrix(glm::value_ptr(rot_attach_a));
    btTransform frame_in_b_tr;
    frame_in_b_tr.setFromOpenGLMatrix(glm::value_ptr(rot_attach_b));

    muscle_slider_constraint = new btSliderConstraint(
        *attach_a->get_body(), *attach_b->get_body(), frame_in_a_tr,
        frame_in_b_tr,
        true);
    muscle_slider_constraint->setEnabled(true);
    muscle_slider_constraint->setPoweredLinMotor(true);
    muscle_slider_constraint->setMaxLinMotorForce(100.f);
    muscle_slider_constraint->setTargetLinMotorVelocity(0.f);

    btTransform frame_in_item_a;
    frame_in_item_a.setFromOpenGLMatrix(glm::value_ptr(translate_in_a * rot_attach_a));
    btTransform frame_in_item_b;
    frame_in_item_b.setFromOpenGLMatrix(glm::value_ptr(translate_in_b * rot_attach_b));

    float limit_angle_cone = 30.f;

    attach_a_constraint = new btConeTwistConstraint(
        *item_a.get_body(),
        *attach_a->get_body(),
        frame_in_item_a,
        frame_in_a_tr
    );
    attach_a_constraint->setEnabled(true);
    attach_a_constraint->setAngularOnly(true);
    attach_a_constraint->setDamping(0.f);
    attach_a_constraint->setLimit(limit_angle_cone, limit_angle_cone, 0.f);

    attach_b_constraint = new btConeTwistConstraint(
        *item_b.get_body(),
        *attach_b->get_body(),
        frame_in_item_b,
        frame_in_b_tr
    );
    attach_b_constraint->setEnabled(true);
    attach_b_constraint->setAngularOnly(true);
    attach_b_constraint->setDamping(0.f);
    attach_b_constraint->setLimit(limit_angle_cone, limit_angle_cone, 0.f);

    item_a.get_body()->setIgnoreCollisionCheck(attach_a->get_body(), true);
    item_b.get_body()->setIgnoreCollisionCheck(attach_a->get_body(), true);

    item_b.get_body()->setIgnoreCollisionCheck(attach_b->get_body(), true);
    item_b.get_body()->setIgnoreCollisionCheck(attach_a->get_body(), true);

    item_a.get_body()->setIgnoreCollisionCheck(item_b.get_body(), true);
}

void Muscle::contract(float force) {
    muscle_slider_constraint->setTargetLinMotorVelocity(-force * 2);
}

void Muscle::release() {

}

std::vector<Item> Muscle::get_items() {
    return {*attach_a, *attach_b};
}

std::vector<btTypedConstraint *> Muscle::get_constraints() {
    return {muscle_slider_constraint, attach_a_constraint, attach_b_constraint};
}
