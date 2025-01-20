//
// Created by samuel on 16/01/25.
//

#include <glm/gtc/type_ptr.hpp>

#include <evo_motion_model/robot/builder.h>

#include "../converter.h"

BuilderMember::BuilderMember(
    const std::string &name, const ShapeKind shapeKind, const glm::vec3 &centerPos,
    const glm::quat &rotation, const glm::vec3 &scale, const float mass, const float friction,
    const bool ignore_collision)
    : Member(name, shapeKind, centerPos, rotation, scale, mass, friction, ignore_collision) {}

BuilderMember::BuilderMember(const std::shared_ptr<AbstractDeserializer> &deserializer)
    : Member(deserializer) {}

void BuilderMember::update_item(
    std::optional<glm::vec3> new_pos, std::optional<glm::quat> new_rot,
    std::optional<glm::vec3> new_scale, std::optional<float> new_friction,
    std::optional<bool> new_ignore_collision) {
    // TODO
}

void BuilderMember::transform_item(
    std::optional<glm::vec3> new_pos, std::optional<glm::quat> new_rot) {
    // TODO
}
