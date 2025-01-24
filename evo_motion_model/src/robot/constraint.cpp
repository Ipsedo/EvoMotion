//
// Created by samuel on 20/01/25.
//

#include <utility>

#include <evo_motion_model/robot/constraint.h>

#include "../converter.h"
#include "../utils.h"

/*
 * Constraint
 */

Constraint::Constraint(
    std::string name, const std::shared_ptr<Member> &parent, const std::shared_ptr<Member> &child)
    : name(std::move(name)), parent(parent), child(child) {}

Constraint::Constraint(
    const std::shared_ptr<AbstractDeserializer> &deserializer,
    const std::function<std::shared_ptr<Member>(std::string)> &get_member_function)
    : Constraint(
          deserializer->read_str("name"),
          get_member_function(deserializer->read_str("parent_name")),
          get_member_function(deserializer->read_str("child_name"))) {}

Constraint::~Constraint() = default;

std::string Constraint::get_name() { return name; }

std::shared_ptr<Member> Constraint::get_parent() { return parent; }

std::shared_ptr<Member> Constraint::get_child() { return child; }

std::shared_ptr<AbstractSerializer>
Constraint::serialize(const std::shared_ptr<AbstractSerializer> &serializer) {
    auto constraint_serializer = serializer->new_object();

    constraint_serializer->write_str("name", name);

    constraint_serializer->write_str("parent_name", get_parent()->get_item().get_name());
    constraint_serializer->write_str("child_name", get_child()->get_item().get_name());

    return constraint_serializer;
}

/*
 * Hinge Constraint
 */

HingeConstraint::HingeConstraint(
    const std::string &name, const std::shared_ptr<Member> &parent,
    const std::shared_ptr<Member> &child, const glm::vec3 &pivot_in_parent,
    const glm::vec3 &pivot_in_child, glm::vec3 axis_in_parent, glm::vec3 axis_in_child,
    const float limit_degree_min, const float limit_degree_max)
    : Constraint(name, parent, child),
      constraint(new btHingeConstraint(
          *parent->get_item().get_body(), *child->get_item().get_body(),
          glm_to_bullet(pivot_in_parent), glm_to_bullet(pivot_in_child),
          glm_to_bullet(axis_in_parent), glm_to_bullet(axis_in_child))),
      min_limit_degree(limit_degree_min), max_limit_degree(limit_degree_max) {

    parent->get_item().get_body()->setIgnoreCollisionCheck(child->get_item().get_body(), true);

    constraint->setLimit(
        static_cast<float>(M_PI) * min_limit_degree / 180.f,
        static_cast<float>(M_PI) * max_limit_degree / 180.f);
    constraint->setOverrideNumSolverIterations(constraint->getOverrideNumSolverIterations() * 8);
}

HingeConstraint::HingeConstraint(
    const std::shared_ptr<AbstractDeserializer> &deserializer,
    const std::function<std::shared_ptr<Member>(std::string)> &get_member_function)
    : HingeConstraint(
          deserializer->read_str("name"),
          get_member_function(deserializer->read_str("parent_name")),
          get_member_function(deserializer->read_str("child_name")),
          deserializer->read_vec3("pivot_in_parent"), deserializer->read_vec3("pivot_in_child"),
          deserializer->read_vec3("axis_in_parent"), deserializer->read_vec3("axis_in_child"),
          deserializer->read_object("limit_degree")->read_float("min"),
          deserializer->read_object("limit_degree")->read_float("max")) {}

btTypedConstraint *HingeConstraint::get_constraint() { return constraint; }

std::shared_ptr<AbstractSerializer>
HingeConstraint::serialize(const std::shared_ptr<AbstractSerializer> &serializer) {
    auto constraint_serializer = Constraint::serialize(serializer);

    /*const auto frame_in_parent = serializer->new_object();
    frame_in_parent->write_vec3("translation", bullet_to_glm(constraint->getFrameOffsetA().getOrigin()));
    frame_in_parent->write_quat("rotation", bullet_to_glm(constraint->getFrameOffsetA().getRotation()));
    constraint_serializer->write_object("frame_in_parent", frame_in_parent);

    const auto frame_in_child = serializer->new_object();
    frame_in_child->write_vec3("translation", bullet_to_glm(constraint->getFrameOffsetB().getOrigin()));
    frame_in_child->write_quat("rotation", bullet_to_glm(constraint->getFrameOffsetB().getRotation()));
    constraint_serializer->write_object("frame_in_child", frame_in_child);*/

    const auto limit_degree_serializer = constraint_serializer->new_object();
    limit_degree_serializer->write_float("min", min_limit_degree);
    limit_degree_serializer->write_float("max", max_limit_degree);
    constraint_serializer->write_object("limit_degree", limit_degree_serializer);

    constraint_serializer->write_vec3(
        "pivot_in_parent", bullet_to_glm(constraint->getFrameOffsetA().getOrigin()));
    constraint_serializer->write_vec3(
        "pivot_in_child", bullet_to_glm(constraint->getFrameOffsetB().getOrigin()));

    constraint_serializer->write_vec3(
        "axis_in_parent", bullet_to_glm(constraint->getFrameOffsetA().getBasis().getColumn(2)));
    constraint_serializer->write_vec3(
        "axis_in_child", bullet_to_glm(constraint->getFrameOffsetB().getBasis().getColumn(2)));

    constraint_serializer->write_str("type", "hinge");

    return constraint_serializer;
}

/*
 * Fixed Constraint
 */

FixedConstraint::FixedConstraint(
    const std::string &name, const std::shared_ptr<Member> &parent,
    const std::shared_ptr<Member> &child, const glm::mat4 &attach_in_parent,
    const glm::mat4 &attach_in_child)
    : Constraint(name, parent, child),
      constraint(new btFixedConstraint(
          *parent->get_item().get_body(), *child->get_item().get_body(),
          glm_to_bullet(attach_in_parent), glm_to_bullet(attach_in_child))) {

    parent->get_item().get_body()->setIgnoreCollisionCheck(child->get_item().get_body(), true);

    constraint->setOverrideNumSolverIterations(constraint->getOverrideNumSolverIterations() * 8);
}

FixedConstraint::FixedConstraint(
    const std::shared_ptr<AbstractDeserializer> &deserializer,
    const std::function<std::shared_ptr<Member>(const std::string &)> &get_member_function)
    : FixedConstraint(
          deserializer->read_str("name"),
          get_member_function(deserializer->read_str("parent_name")),
          get_member_function(deserializer->read_str("child_name")),
          glm::translate(
              glm::mat4(1.f),
              deserializer->read_object("frame_in_parent")->read_vec3("translation"))
              * glm::toMat4(deserializer->read_object("frame_in_parent")->read_quat("rotation")),
          glm::translate(
              glm::mat4(1.f), deserializer->read_object("frame_in_child")->read_vec3("translation"))
              * glm::toMat4(deserializer->read_object("frame_in_child")->read_quat("rotation"))) {}

btTypedConstraint *FixedConstraint::get_constraint() { return constraint; }

std::shared_ptr<AbstractSerializer>
FixedConstraint::serialize(const std::shared_ptr<AbstractSerializer> &serializer) {
    auto constraint_serializer = Constraint::serialize(serializer);

    const auto frame_in_parent = serializer->new_object();
    frame_in_parent->write_vec3(
        "translation", bullet_to_glm(constraint->getFrameOffsetA().getOrigin()));
    frame_in_parent->write_quat(
        "rotation", bullet_to_glm(constraint->getFrameOffsetA().getRotation()));
    constraint_serializer->write_object("frame_in_parent", frame_in_parent);

    const auto frame_in_child = serializer->new_object();
    frame_in_child->write_vec3(
        "translation", bullet_to_glm(constraint->getFrameOffsetB().getOrigin()));
    frame_in_child->write_quat(
        "rotation", bullet_to_glm(constraint->getFrameOffsetB().getRotation()));
    constraint_serializer->write_object("frame_in_child", frame_in_child);

    constraint_serializer->write_str("type", "fixed");

    return constraint_serializer;
}