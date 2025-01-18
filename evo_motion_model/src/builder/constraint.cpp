//
// Created by samuel on 15/01/25.
//

#include <glm/gtc/type_ptr.hpp>

#include <evo_motion_model/robot_builder.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include "../converter.h"

/*
 * Builder Constraint
 */

BuilderConstraint::BuilderConstraint(const std::shared_ptr<BuilderMember> &child) : child(child) {}

std::shared_ptr<BuilderMember> BuilderConstraint::get_builder_member_child() { return child; }

/*
 * Builder Fixed Constraint
 */

BuilderFixedConstraint::BuilderFixedConstraint(
    const Item &parent, const glm::mat4 &attach_in_parent, const glm::mat4 &attach_in_child,
    const std::shared_ptr<BuilderMember> &child)
    : BuilderConstraint(child),
      constraint(new btFixedConstraint(
          *parent.get_body(), *child->get_item().get_body(), glm_to_bullet(attach_in_parent),
          glm_to_bullet(attach_in_child))) {

    constraint->setOverrideNumSolverIterations(constraint->getOverrideNumSolverIterations() * 8);

    parent.get_body()->setIgnoreCollisionCheck(child->get_item().get_body(), true);
}
btTypedConstraint *BuilderFixedConstraint::get_constraint() { return constraint; }
std::shared_ptr<AbstractMember> BuilderFixedConstraint::get_child() { return child; }

/*
 * Hinge Builder Constraint
 */

BuilderHingeConstraint::BuilderHingeConstraint(
    const Item &parent, const glm::mat4 frame_in_parent, const glm::mat4 frame_in_child,
    float limit_degree_min, float limit_degree_max, const std::shared_ptr<BuilderMember> &child)
    : BuilderConstraint(child),
      constraint(new btHingeConstraint(
          *parent.get_body(), *child->get_item().get_body(), glm_to_bullet(frame_in_parent),
          glm_to_bullet(frame_in_child))) {

    constraint->setLimit(M_PI * limit_degree_min / 180.f, M_PI * limit_degree_max / 180.f);

    constraint->setOverrideNumSolverIterations(constraint->getOverrideNumSolverIterations() * 8);

    parent.get_body()->setIgnoreCollisionCheck(child->get_item().get_body(), true);
}

btTypedConstraint *BuilderHingeConstraint::get_constraint() { return constraint; }

std::shared_ptr<AbstractMember> BuilderHingeConstraint::get_child() {
    return get_builder_member_child();
}
