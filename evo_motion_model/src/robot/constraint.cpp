//
// Created by samuel on 20/01/25.
//

#include <evo_motion_model/robot/constraint.h>

#include <utility>

#include "../converter.h"
#include "../utils.h"

/*
 * Constraint
 */

Constraint::Constraint(
    std::string name, const std::shared_ptr<Member> &parent,
    const std::shared_ptr<Member> &child)
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
    auto serializer_constraint = serializer->new_object();

    serializer_constraint->write_str("parent_name", get_parent()->get_item().get_name());
    serializer_constraint->write_str("child_name", get_child()->get_item().get_name());

    return serializer_constraint;
}

/*
 * Hinge Constraint
 */

HingeConstraint::HingeConstraint(
    const std::string &name, const std::shared_ptr<Member> &parent,
    const std::shared_ptr<Member> &child, const glm::mat4 &frame_in_parent,
    const glm::mat4 &frame_in_child, const float limit_degree_min, const float limit_degree_max)
    : Constraint(name, parent, child),
      constraint(new btHingeConstraint(
          *parent->get_item().get_body(), *child->get_item().get_body(),
          glm_to_bullet(frame_in_parent), glm_to_bullet(frame_in_child))) {

    constraint->setLimit(M_PI * limit_degree_min / 180.f, M_PI * limit_degree_max / 180.f);
    constraint->setOverrideNumSolverIterations(constraint->getOverrideNumSolverIterations() * 8);

    parent->get_item().get_body()->setIgnoreCollisionCheck(child->get_item().get_body(), true);
}

HingeConstraint::HingeConstraint(
    const std::shared_ptr<AbstractDeserializer> &deserializer,
    const std::function<std::shared_ptr<Member>(std::string)> &get_member_function)
    : HingeConstraint(
          deserializer->read_str("name"),
          get_member_function(deserializer->read_str("parent_name")),
          get_member_function(deserializer->read_str("child_name")),
          glm::translate(
              glm::mat4(1.f),
              deserializer->read_object("frame_in_parent")->read_vec3("translation"))
              * glm::toMat4(deserializer->read_object("frame_in_parent")->read_quat("rotation")),
          glm::translate(
              glm::mat4(1.f), deserializer->read_object("frame_in_child")->read_vec3("translation"))
              * glm::toMat4(deserializer->read_object("frame_in_child")->read_quat("rotation")),
          deserializer->read_object("limit_degree")->read_float("min"),
          deserializer->read_object("limit_degree")->read_float("max")) {}

btTypedConstraint *HingeConstraint::get_constraint() { return constraint; }

std::shared_ptr<AbstractSerializer>
HingeConstraint::serialize(const std::shared_ptr<AbstractSerializer> &serializer) {
    auto serializer_constraint = Constraint::serialize(serializer);

    const auto frame_in_parent = serializer->new_object();
    frame_in_parent->write_vec3("translation", bullet_to_glm(constraint->getAFrame().getOrigin()));
    frame_in_parent->write_quat("rotation", bullet_to_glm(constraint->getAFrame().getRotation()));
    serializer_constraint->write_object("frame_in_parent", frame_in_parent);

    const auto frame_in_child = serializer->new_object();
    frame_in_child->write_vec3("translation", bullet_to_glm(constraint->getBFrame().getOrigin()));
    frame_in_child->write_quat("rotation", bullet_to_glm(constraint->getBFrame().getRotation()));
    serializer_constraint->write_object("frame_in_child", frame_in_parent);

    const auto limit_degree_serializer = serializer_constraint->new_object();
    limit_degree_serializer->write_float(
        "min", constraint->getLowerLimit() * 180.f / static_cast<float>(M_PI));
    limit_degree_serializer->write_float(
        "max", constraint->getUpperLimit() * 180.f / static_cast<float>(M_PI));
    serializer_constraint->write_object("limit_degree", limit_degree_serializer);

    return serializer_constraint;
}

/*
 * Fixed Constraint
 */

FixedConstraint::FixedConstraint(
    const std::string &name, const std::shared_ptr<Member> &parent,
    const std::shared_ptr<Member> &child, const glm::mat4 &attach_in_parent, const glm::mat4 &attach_in_child)
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
    constraint_serializer->write_object("frame_in_child", frame_in_parent);

    return constraint_serializer;
}