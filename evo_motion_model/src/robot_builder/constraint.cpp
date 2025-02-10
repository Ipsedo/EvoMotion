//
// Created by samuel on 15/01/25.
//

#include <glm/gtc/type_ptr.hpp>

#include <evo_motion_model/robot/builder.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include "../converter.h"

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

/*
 * Hinge
 */

BuilderHingeConstraint::BuilderHingeConstraint(
    const std::string &name, const std::shared_ptr<Member> &parent,
    const std::shared_ptr<Member> &child, const glm::vec3 &pivot_in_parent,
    const glm::vec3 &pivot_in_child, glm::vec3 axis_in_parent, glm::vec3 axis_in_child,
    float limit_radian_min, float limit_radian_max)
    : HingeConstraint(
          name, parent, child, pivot_in_parent, pivot_in_child, axis_in_parent, axis_in_child,
          limit_radian_min, limit_radian_max),
      BuilderConstraint(name, parent, child), Constraint(name, parent, child), shape(std::make_shared<ObjShape>("./resources/obj/cylinder.obj")) {}

BuilderHingeConstraint::BuilderHingeConstraint(
    const std::shared_ptr<AbstractDeserializer> &deserializer,
    const std::function<std::shared_ptr<Member>(std::string)> &get_member_function)
    : HingeConstraint(deserializer, get_member_function),
      BuilderConstraint(deserializer, get_member_function),
      Constraint(deserializer, get_member_function), shape(std::make_shared<ObjShape>("./resources/obj/cylinder.obj")) {}

void BuilderHingeConstraint::update_constraint(
    const std::optional<glm::vec3> &pivot_in_parent, const std::optional<glm::vec3> &pivot_in_child,
    std::optional<glm::vec3> axis_in_parent, std::optional<glm::vec3> axis_in_child,
    std::optional<float> limit_radian_min, std::optional<float> limit_radian_max) {}

Item BuilderHingeConstraint::get_fake_item(){
    const auto [pos, rot, scale] = get_empty_item_transform();
    return Item(get_name(), shape, pos, rot, scale, 0.f, SPECULAR);
}

/*
 * Fixed
 */

BuilderFixedConstraint::BuilderFixedConstraint(
    const std::string &name, const std::shared_ptr<Member> &parent,
    const std::shared_ptr<Member> &child, const glm::mat4 &frame_in_parent,
    const glm::mat4 &frame_in_child)
    : FixedConstraint(name, parent, child, frame_in_parent, frame_in_child),
      BuilderConstraint(name, parent, child), Constraint(name, parent, child), shape(std::make_shared<ObjShape>("./resources/obj/cube.obj")) {}

BuilderFixedConstraint::BuilderFixedConstraint(
    const std::shared_ptr<AbstractDeserializer> &deserializer,
    const std::function<std::shared_ptr<Member>(std::string)> &get_member_function)
    : FixedConstraint(deserializer, get_member_function),
      BuilderConstraint(deserializer, get_member_function),
      Constraint(deserializer, get_member_function), shape(std::make_shared<ObjShape>("./resources/obj/cube.obj")) {}

Item BuilderFixedConstraint::get_fake_item() {
    const auto [pos, rot, scale] = get_empty_item_transform();
    return Item(get_name(), shape, pos, rot, scale, 0.f, SPECULAR);
}
