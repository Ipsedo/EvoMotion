//
// Created by samuel on 30/12/23.
//

#include <iostream>
#include <memory>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <evo_motion_model/robot/muscle.h>

#include "../converter.h"

glm::mat4 get_rotation(const glm::vec3 a, const glm::vec3 b) {
    return glm::rotate(
        glm::mat4(1.0f), acos(glm::dot(b, a) / (glm::length(b) * glm::length(a))),
        glm::cross(b, a));
}

Muscle::Muscle(
    const std::string &name, const float attach_mass, const glm::vec3 attach_scale,
    const std::shared_ptr<Item> &item_a, const glm::vec3 pos_in_a,
    const std::shared_ptr<Item> &item_b, const glm::vec3 pos_in_b, const float force,
    const float max_speed)
    : name(name), item_a_name(item_a->get_name()), item_b_name(item_b->get_name()),
      max_speed(max_speed),
      attach_a(std::make_shared<Item>(
          name + "_attach_a", std::make_shared<ObjShape>("./resources/obj/sphere.obj"),
          item_a->model_matrix_without_scale() * glm::translate(glm::mat4(1), pos_in_a),
          attach_scale, attach_mass, SPECULAR)),
      attach_b(std::make_shared<Item>(
          name + "_attach_b", std::make_shared<ObjShape>("./resources/obj/sphere.obj"),
          item_b->model_matrix_without_scale() * glm::translate(glm::mat4(1), pos_in_b),
          attach_scale, attach_mass, SPECULAR)) {

    btTransform frame_in_attach_a;
    frame_in_attach_a.setIdentity();
    btTransform frame_in_attach_b;
    frame_in_attach_b.setIdentity();

    muscle_slider_constraint = new btSliderConstraint(
        *attach_a->get_body(), *attach_b->get_body(), frame_in_attach_a, frame_in_attach_b, true);

    muscle_slider_constraint->setMaxLinMotorForce(force);

    muscle_slider_constraint->setLowerAngLimit(0);
    muscle_slider_constraint->setUpperAngLimit(0);

    muscle_slider_constraint->setLowerLinLimit(0);
    const float max_extension_muscle =
        2.f
        * glm::length(glm::vec3(
            attach_a->model_matrix_without_scale() * glm::vec4(glm::vec3(0), 1)
            - attach_b->model_matrix_without_scale() * glm::vec4(glm::vec3(0), 1)));
    muscle_slider_constraint->setUpperLinLimit(max_extension_muscle);

    attach_a_constraint = new btPoint2PointConstraint(
        *item_a->get_body(), *attach_a->get_body(), glm_to_bullet(pos_in_a), btVector3(0, 0, 0));

    attach_b_constraint = new btPoint2PointConstraint(
        *item_b->get_body(), *attach_b->get_body(), glm_to_bullet(pos_in_b), btVector3(0, 0, 0));

    attach_a->get_body()->setCollisionFlags(
        attach_a->get_body()->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
    attach_b->get_body()->setCollisionFlags(
        attach_b->get_body()->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);

    attach_a_constraint->setOverrideNumSolverIterations(
        attach_a_constraint->getOverrideNumSolverIterations() * 4);
    attach_b_constraint->setOverrideNumSolverIterations(
        attach_b_constraint->getOverrideNumSolverIterations() * 4);
    muscle_slider_constraint->setOverrideNumSolverIterations(
        muscle_slider_constraint->getOverrideNumSolverIterations() * 4);
}

Muscle::Muscle(
    const std::shared_ptr<AbstractDeserializer> &deserializer,
    const std::function<std::shared_ptr<Member>(std::string)> &get_member_function)
    : Muscle(
          deserializer->read_str("name"), deserializer->read_float("attach_mass"),
          deserializer->read_vec3("attach_scale"),
          get_member_function(deserializer->read_str("item_a"))->get_item(),
          deserializer->read_vec3("pos_in_a"),
          get_member_function(deserializer->read_str("item_b"))->get_item(),
          deserializer->read_vec3("pos_in_b"), deserializer->read_float("force"),
          deserializer->read_float("speed")) {}

void Muscle::contract(const float speed_factor) const {
    muscle_slider_constraint->setPoweredLinMotor(true);
    muscle_slider_constraint->setTargetLinMotorVelocity(speed_factor * max_speed);
}

void Muscle::release() const { muscle_slider_constraint->setPoweredLinMotor(false); }

std::string Muscle::get_name() { return name; }

std::vector<std::shared_ptr<AbstractItem>> Muscle::get_items() { return {attach_a, attach_b}; }

std::vector<btRigidBody *> Muscle::get_bodies() {
    return {attach_a->get_body(), attach_b->get_body()};
}

std::vector<btTypedConstraint *> Muscle::get_constraints() {
    return {muscle_slider_constraint, attach_a_constraint, attach_b_constraint};
}

btSliderConstraint *Muscle::get_slider_constraint() const { return muscle_slider_constraint; }

std::tuple<btPoint2PointConstraint *, btPoint2PointConstraint *> Muscle::get_p2p_constraints() {
    return {attach_a_constraint, attach_b_constraint};
}

std::shared_ptr<AbstractSerializer>
Muscle::serialize(const std::shared_ptr<AbstractSerializer> &serializer) {
    auto muscle_serializer = serializer->new_object();

    muscle_serializer->write_str("name", name);

    muscle_serializer->write_float("attach_mass", attach_a->get_body()->getMass());
    muscle_serializer->write_vec3(
        "attach_scale",
        bullet_to_glm(attach_a->get_body()->getCollisionShape()->getLocalScaling()));
    muscle_serializer->write_str("item_a", item_a_name);
    muscle_serializer->write_str("item_b", item_b_name);

    muscle_serializer->write_vec3("pos_in_a", bullet_to_glm(attach_a_constraint->getPivotInA()));
    muscle_serializer->write_vec3("pos_in_b", bullet_to_glm(attach_b_constraint->getPivotInA()));

    muscle_serializer->write_float("force", muscle_slider_constraint->getMaxLinMotorForce());
    muscle_serializer->write_float("speed", max_speed);

    return muscle_serializer;
}

Muscle::~Muscle() = default;
