//
// Created by samuel on 15/01/25.
//

#include <glm/gtc/type_ptr.hpp>

#include <evo_motion_model/robot/builder.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include <evo_motion_model/converter.h>

/*
 * Builder Constraint
 */

BuilderConstraint::BuilderConstraint(
    const std::string &name, const std::shared_ptr<Member> &parent,
    const std::shared_ptr<Member> &child)
    : Constraint(name, parent, child) {}

BuilderConstraint::BuilderConstraint(
    const std::shared_ptr<AbstractDeserializer> &deserializer,
    const std::function<std::shared_ptr<Member>(std::string)> &get_member_function)
    : Constraint(deserializer, get_member_function) {}

btRigidBody *BuilderConstraint::create_fake_body() {
    const auto [pos, rot, scale] = get_empty_item_transform();

    auto *convex_hull_shape = new btConvexHullShape();

    for (auto [x, y, z]: get_shape()->get_vertices())
        convex_hull_shape->addPoint(btVector3(x, y, z));

    convex_hull_shape->setLocalScaling(glm_to_bullet(scale));

    btTransform original_tr;
    original_tr.setOrigin(glm_to_bullet(pos));
    original_tr.setRotation(glm_to_bullet(rot));

    auto *motion_state = new btDefaultMotionState(original_tr);

    const btRigidBody::btRigidBodyConstructionInfo body_info(
        0.f, motion_state, convex_hull_shape, btVector3(0, 0, 0));

    return new btRigidBody(body_info);
}

/*
 * Hinge
 */

BuilderHingeConstraint::BuilderHingeConstraint(
    const std::string &name, const std::shared_ptr<Member> &parent,
    const std::shared_ptr<Member> &child, const glm::vec3 &pivot_in_parent,
    const glm::vec3 &pivot_in_child, const glm::vec3 &axis_in_parent,
    const glm::vec3 &axis_in_child, const float limit_radian_min, const float limit_radian_max)
    : HingeConstraint(
          name, parent, child, pivot_in_parent, pivot_in_child, axis_in_parent, axis_in_child,
          limit_radian_min, limit_radian_max),
      Constraint(name, parent, child), BuilderConstraint(name, parent, child),
      shape(std::make_shared<ObjShape>("./resources/obj/cylinder.obj")) {}

BuilderHingeConstraint::BuilderHingeConstraint(
    const std::shared_ptr<AbstractDeserializer> &deserializer,
    const std::function<std::shared_ptr<Member>(std::string)> &get_member_function)
    : HingeConstraint(deserializer, get_member_function),
      Constraint(deserializer, get_member_function),
      BuilderConstraint(deserializer, get_member_function),
      shape(std::make_shared<ObjShape>("./resources/obj/cylinder.obj")) {}

void BuilderHingeConstraint::update_constraint(
    const std::optional<glm::vec3> &new_pivot, const std::optional<glm::vec3> &new_axis,
    const std::optional<float> &new_limit_radian_min,
    const std::optional<float> &new_limit_radian_max) {

    // Frames
    const auto parent_model_mat = get_parent()->get_item()->model_matrix_without_scale();
    const auto frame_in_parent = bullet_to_glm(constraint->getFrameOffsetA());
    // force to use parent absolute pos (from parent frame)
    const auto absolute_model_mat = parent_model_mat * frame_in_parent;

    const auto child_model_mat = get_child()->get_item()->model_matrix_without_scale();

    const auto absolute_pivot =
        new_pivot.has_value() ? new_pivot.value() : glm::vec3(absolute_model_mat[3]);
    const auto absolute_axis =
        new_axis.has_value() ? new_axis.value() : glm::mat3(absolute_model_mat)[2];

    const glm::mat4 new_absolute_frame =
        glm::translate(glm::mat4(1.f), absolute_pivot)
        * glm::toMat4(glm::rotation(glm::vec3(0, 0, 1), glm::normalize(absolute_axis)));

    const glm::mat4 new_frame_in_parent = glm::inverse(parent_model_mat) * new_absolute_frame;
    const glm::mat4 new_frame_in_child = glm::inverse(child_model_mat) * new_absolute_frame;

    constraint->setFrames(glm_to_bullet(new_frame_in_parent), glm_to_bullet(new_frame_in_child));

    // Limit
    constraint->setLimit(
        new_limit_radian_min.has_value() ? new_limit_radian_min.value()
                                         : constraint->getLowerLimit(),
        new_limit_radian_max.has_value() ? new_limit_radian_max.value()
                                         : constraint->getUpperLimit());
}

std::shared_ptr<Shape> BuilderHingeConstraint::get_shape() { return shape; }

/*
 * Fixed
 */

BuilderFixedConstraint::BuilderFixedConstraint(
    const std::string &name, const std::shared_ptr<Member> &parent,
    const std::shared_ptr<Member> &child, const glm::mat4 &frame_in_parent,
    const glm::mat4 &frame_in_child)
    : FixedConstraint(name, parent, child, frame_in_parent, frame_in_child),
      Constraint(name, parent, child), BuilderConstraint(name, parent, child),
      shape(std::make_shared<ObjShape>("./resources/obj/cube.obj")) {}

BuilderFixedConstraint::BuilderFixedConstraint(
    const std::shared_ptr<AbstractDeserializer> &deserializer,
    const std::function<std::shared_ptr<Member>(std::string)> &get_member_function)
    : FixedConstraint(deserializer, get_member_function),
      Constraint(deserializer, get_member_function),
      BuilderConstraint(deserializer, get_member_function),
      shape(std::make_shared<ObjShape>("./resources/obj/cube.obj")) {}

std::shared_ptr<Shape> BuilderFixedConstraint::get_shape() { return shape; }

void BuilderFixedConstraint::update_constraint(
    const std::optional<glm::vec3> &new_pivot, const std::optional<glm::quat> &new_rot) {
    const auto parent_model_mat = get_parent()->get_item()->model_matrix_without_scale();
    const auto frame_in_parent = bullet_to_glm(constraint->getFrameOffsetA());
    // force to use parent absolute pos (from parent frame)
    const auto absolute_model_mat = parent_model_mat * frame_in_parent;

    const auto child_model_mat = get_child()->get_item()->model_matrix_without_scale();

    const auto [old_pos, old_rot, _] = decompose_model_matrix(absolute_model_mat);

    const auto absolute_pivot = new_pivot.has_value() ? new_pivot.value() : old_pos;
    const auto absolute_rot = new_rot.has_value() ? new_rot.value() : old_rot;

    const glm::mat4 new_absolute_frame =
        glm::translate(glm::mat4(1.f), absolute_pivot) * glm::toMat4(absolute_rot);

    const glm::mat4 new_frame_in_parent = glm::inverse(parent_model_mat) * new_absolute_frame;
    const glm::mat4 new_frame_in_child = glm::inverse(child_model_mat) * new_absolute_frame;

    constraint->setFrames(glm_to_bullet(new_frame_in_parent), glm_to_bullet(new_frame_in_child));
}
