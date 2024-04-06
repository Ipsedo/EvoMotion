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
#include "./converter.h"


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
    glm::vec3 pos_in_b,
    float force,
    float max_speed
) : max_speed(max_speed),
    attach_a(name + "_attach_a",
             std::make_shared<ObjShape>(
                 "./resources/obj/sphere.obj"),
             item_a.model_matrix_without_scale() *
             glm::translate(glm::mat4(1), pos_in_a),
             attach_scale,
             attach_mass),
    attach_b(name + "_attach_b",
             std::make_shared<ObjShape>(
                 "./resources/obj/sphere.obj"),
             item_b.model_matrix_without_scale() *
             glm::translate(glm::mat4(1), pos_in_b),
             attach_scale,
             attach_mass) {


    btTransform frame_in_attach_a;
    frame_in_attach_a.setIdentity();
    btTransform frame_in_attach_b;
    frame_in_attach_b.setIdentity();

    muscle_slider_constraint = new btSliderConstraint(
        *attach_a.get_body(), *attach_b.get_body(), frame_in_attach_a,
        frame_in_attach_b,
        true);

    muscle_slider_constraint->setMaxLinMotorForce(force);
    muscle_slider_constraint->setTargetLinMotorVelocity(0.f);

    muscle_slider_constraint->setLowerAngLimit(0);
    muscle_slider_constraint->setUpperAngLimit(0);
    muscle_slider_constraint->setLowerLinLimit(0);

    muscle_slider_constraint->setSoftnessDirLin(0);
    muscle_slider_constraint->setSoftnessDirAng(0);

    attach_a_constraint = new btPoint2PointConstraint(
        *item_a.get_body(),
        *attach_a.get_body(),
        glm_to_bullet(pos_in_a),
        btVector3(0, 0, 0)
    );

    attach_b_constraint = new btPoint2PointConstraint(
        *item_b.get_body(),
        *attach_b.get_body(),
        glm_to_bullet(pos_in_b),
        btVector3(0, 0, 0)
    );

    /*for (int i = 0; i < 6; i++) {
        attach_a_constraint->setParam(BT_CONSTRAINT_STOP_CFM, 0, i);
        attach_b_constraint->setParam(BT_CONSTRAINT_STOP_CFM, 0, i);
        muscle_slider_constraint->setParam(BT_CONSTRAINT_STOP_CFM, 0, i);

        attach_a_constraint->setParam(BT_CONSTRAINT_STOP_ERP, 1, i);
        attach_b_constraint->setParam(BT_CONSTRAINT_STOP_ERP, 1, i);
        muscle_slider_constraint->setParam(BT_CONSTRAINT_STOP_ERP, 1, i);
    }*/

    attach_a.get_body()->setCollisionFlags(
        attach_a.get_body()->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
    attach_b.get_body()->setCollisionFlags(
        attach_b.get_body()->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
}

void Muscle::contract(float force) {
    muscle_slider_constraint->setPoweredLinMotor(true);
    muscle_slider_constraint->setTargetLinMotorVelocity(force * max_speed);
}

void Muscle::release() {
    muscle_slider_constraint->setPoweredLinMotor(false);
}

std::vector<Item> Muscle::get_items() {
    return {attach_a, attach_b};
}

std::vector<btTypedConstraint *> Muscle::get_constraints() {
    return {muscle_slider_constraint, attach_a_constraint, attach_b_constraint};
}

/*
 * JSON
 */

JsonMuscularSystem::JsonMuscularSystem(Skeleton skeleton, const std::string &json_path) : muscles() {
    auto json_muscles = read_json(json_path)["muscles"];

    for (auto json_muscle: json_muscles) {
        std::string item_a_name = json_muscle["item_a"].get<std::string>();
        std::string item_b_name = json_muscle["item_b"].get<std::string>();
        Item item_a = skeleton.get_item(skeleton.get_root_name() + "_" + item_a_name);
        Item item_b = skeleton.get_item(skeleton.get_root_name() + "_" + item_b_name);

        muscles.emplace_back(
            skeleton.get_root_name().append("_").append(json_muscle["name"].get<std::string>()),
            json_muscle["attach_mass"].get<float>(),
            json_vec3_to_glm_vec3(json_muscle["attach_scale"]),
            item_a, json_vec3_to_glm_vec3(json_muscle["pos_in_a"]),
            item_b, json_vec3_to_glm_vec3(json_muscle["pos_in_b"]),
            json_muscle["force"].get<float>(),
            json_muscle["speed"].get<float>()
        );
    }
}

std::vector<Muscle> JsonMuscularSystem::get_muscles() {
    return muscles;
}
