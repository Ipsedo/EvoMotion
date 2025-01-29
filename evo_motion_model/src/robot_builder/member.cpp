//
// Created by samuel on 16/01/25.
//

#include <glm/gtc/type_ptr.hpp>

#include <evo_motion_model/robot/builder.h>

#include "../converter.h"

BuilderMember::BuilderMember(
    const std::string &name, const ShapeKind shape_kind, const glm::vec3 &center_pos,
    const glm::quat &rotation, const glm::vec3 &scale, const float mass, const float friction,
    const bool ignore_collision)
    : Member(name, shape_kind, center_pos, rotation, scale, mass, friction, ignore_collision) {}

BuilderMember::BuilderMember(const std::shared_ptr<AbstractDeserializer> &deserializer)
    : Member(deserializer) {}

void BuilderMember::update_item(
    std::optional<glm::vec3> new_pos, std::optional<glm::quat> new_rot,
    std::optional<glm::vec3> new_scale, std::optional<float> new_friction,
    std::optional<bool> new_ignore_collision) {

    const auto translate =
        new_pos.has_value() ? glm::translate(glm::mat4(1.f), new_pos.value()) : glm::mat4(1.f);
    const auto rotation = new_pos.has_value() ? glm::toMat4(new_rot.value()) : glm::mat4(1.f);

    const auto model_matrix = translate * rotation;

    get_item().get_body()->setWorldTransform(glm_to_bullet(model_matrix));

    if (new_scale.has_value())
        get_item().get_body()->getCollisionShape()->setLocalScaling(
            glm_to_bullet(new_scale.value()));

    if (new_friction.has_value()) get_item().get_body()->setFriction(new_friction.value());

    // TODO ignore collision
}
