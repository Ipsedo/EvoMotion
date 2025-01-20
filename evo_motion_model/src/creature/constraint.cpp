//
// Created by samuel on 20/01/25.
//

#include <evo_motion_model/constraint.h>

#include "../converter.h"
#include "../utils.h"

/*
 * Constraint
 */

NewConstraint::NewConstraint(
    const std::shared_ptr<NewMember> &parent, const std::shared_ptr<NewMember> &child)
    : parent(parent), child(child) {}

NewConstraint::NewConstraint(
    const std::shared_ptr<AbstractDeserializer> &deserializer,
    const std::function<std::shared_ptr<NewMember>(std::string)> &get_member_function)
    : parent(get_member_function(deserializer->read_str("parent_name"))),
      child(get_member_function(deserializer->read_str("child_name"))) {}

NewConstraint::~NewConstraint() = default;

std::shared_ptr<NewMember> NewConstraint::get_parent() { return parent; }

std::shared_ptr<NewMember> NewConstraint::get_child() { return child; }

std::shared_ptr<AbstractSerializer<std::any>>
NewConstraint::serialize(const std::shared_ptr<AbstractSerializer<std::any>> &serializer) {
    auto serializer_constraint = serializer->new_object();

    serializer_constraint->write_str("parent_name", get_parent()->get_item().get_name());
    serializer_constraint->write_str("child_name", get_child()->get_item().get_name());

    return serializer_constraint;
}

/*
 * Hinge Constraint
 */

NewHingeConstraint::NewHingeConstraint(
    const std::shared_ptr<NewMember> &parent, const std::shared_ptr<NewMember> &child,
    const glm::mat4 frame_in_parent, const glm::mat4 frame_in_child, float limit_degree_min,
    float limit_degree_max)
    : NewConstraint(parent, child),
      constraint(new btHingeConstraint(
          *parent->get_item().get_body(), *child->get_item().get_body(),
          glm_to_bullet(frame_in_parent), glm_to_bullet(frame_in_child))) {

    constraint->setLimit(M_PI * limit_degree_min / 180.f, M_PI * limit_degree_max / 180.f);
    constraint->setOverrideNumSolverIterations(constraint->getOverrideNumSolverIterations() * 8);

    parent->get_item().get_body()->setIgnoreCollisionCheck(child->get_item().get_body(), true);
}

NewHingeConstraint::NewHingeConstraint(
    const std::shared_ptr<AbstractDeserializer> &deserializer,
    const std::function<std::shared_ptr<NewMember>(std::string)> &get_member_function)
    : NewHingeConstraint(
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

btTypedConstraint *NewHingeConstraint::get_constraint() { return constraint; }

std::shared_ptr<AbstractSerializer<std::any>>
NewHingeConstraint::serialize(const std::shared_ptr<AbstractSerializer<std::any>> &serializer) {
    auto serializer_constraint = NewConstraint::serialize(serializer);

    auto frame_in_parent = serializer->new_object();
    frame_in_parent->write_vec3("translation", bullet_to_glm(constraint->getAFrame().getOrigin()));
    frame_in_parent->write_quat("rotation", bullet_to_glm(constraint->getAFrame().getRotation()));
    serializer_constraint->write_object("frame_in_parent", frame_in_parent);

    auto frame_in_child = serializer->new_object();
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

NewFixedConstraint::NewFixedConstraint(
    const std::shared_ptr<NewMember> &parent, const std::shared_ptr<NewMember> &child,
    glm::mat4 attach_in_parent, glm::mat4 attach_in_child)
    : NewConstraint(parent, child),
      constraint(new btFixedConstraint(
          *parent->get_item().get_body(), *child->get_item().get_body(),
          glm_to_bullet(attach_in_parent), glm_to_bullet(attach_in_child))) {

    parent->get_item().get_body()->setIgnoreCollisionCheck(child->get_item().get_body(), true);

    constraint->setOverrideNumSolverIterations(constraint->getOverrideNumSolverIterations() * 8);
}

NewFixedConstraint::NewFixedConstraint(
    const std::shared_ptr<AbstractDeserializer> &deserializer,
    const std::function<std::shared_ptr<NewMember>(const std::string &)> &get_member_function)
    : NewFixedConstraint(
          get_member_function(deserializer->read_str("parent_name")),
          get_member_function(deserializer->read_str("child_name")),
          glm::translate(
              glm::mat4(1.f),
              deserializer->read_object("frame_in_parent")->read_vec3("translation"))
              * glm::toMat4(deserializer->read_object("frame_in_parent")->read_quat("rotation")),
          glm::translate(
              glm::mat4(1.f), deserializer->read_object("frame_in_child")->read_vec3("translation"))
              * glm::toMat4(deserializer->read_object("frame_in_child")->read_quat("rotation"))) {}

btTypedConstraint *NewFixedConstraint::get_constraint() { return constraint; }

std::shared_ptr<AbstractSerializer<std::any>>
NewFixedConstraint::serialize(const std::shared_ptr<AbstractSerializer<std::any>> &serializer) {
    auto constraint_serializer = NewConstraint::serialize(serializer);

    auto frame_in_parent = serializer->new_object();
    frame_in_parent->write_vec3(
        "translation", bullet_to_glm(constraint->getFrameOffsetA().getOrigin()));
    frame_in_parent->write_quat(
        "rotation", bullet_to_glm(constraint->getFrameOffsetA().getRotation()));
    constraint_serializer->write_object("frame_in_parent", frame_in_parent);

    auto frame_in_child = serializer->new_object();
    frame_in_child->write_vec3(
        "translation", bullet_to_glm(constraint->getFrameOffsetB().getOrigin()));
    frame_in_child->write_quat(
        "rotation", bullet_to_glm(constraint->getFrameOffsetB().getRotation()));
    constraint_serializer->write_object("frame_in_child", frame_in_parent);

    return constraint_serializer;
}