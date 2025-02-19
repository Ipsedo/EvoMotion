//
// Created by samuel on 16/01/25.
//

#include <glm/gtc/type_ptr.hpp>

#include <evo_motion_model/converter.h>
#include <evo_motion_model/robot/builder.h>

BuilderMember::BuilderMember(
    const std::string &name, const ShapeKind shape_kind, const glm::vec3 &center_pos,
    const glm::quat &rotation, const glm::vec3 &scale, const float mass, const float friction,
    const bool ignore_collision)
    : Member(name, shape_kind, center_pos, rotation, scale, mass, friction, ignore_collision) {}

BuilderMember::BuilderMember(const std::shared_ptr<AbstractDeserializer> &deserializer)
    : Member(deserializer) {}

void BuilderMember::update_item(
    const std::optional<glm::vec3> &new_pos, const std::optional<glm::quat> &new_rot,
    const std::optional<glm::vec3> &new_scale, const std::optional<float> &new_friction,
    const std::optional<float> &new_mass, const std::optional<bool> &new_ignore_collision) {

    const auto [pos, rot, _] = decompose_model_matrix(get_item()->model_matrix_without_scale());

    const auto translate =
        glm::translate(glm::mat4(1.f), new_pos.has_value() ? new_pos.value() : pos);
    const auto rotation = glm::toMat4(new_rot.has_value() ? new_rot.value() : rot);

    const auto model_matrix = translate * rotation;

    get_item()->get_body()->setWorldTransform(glm_to_bullet(model_matrix));
    get_item()->get_body()->getMotionState()->setWorldTransform(glm_to_bullet(model_matrix));

    if (new_scale.has_value()) {
        const auto shape = get_item()->get_body()->getCollisionShape();
        shape->setLocalScaling(glm_to_bullet(new_scale.value()));

        btVector3 local_inertia(0, 0, 0);
        shape->calculateLocalInertia(get_item()->get_body()->getMass(), local_inertia);

        get_item()->get_body()->updateInertiaTensor();
    }

    if (new_friction.has_value()) get_item()->get_body()->setFriction(new_friction.value());
    if (new_mass.has_value()) {
        get_item()->get_body()->setMassProps(
            new_mass.value(), get_item()->get_body()->getLocalInertia());
        get_item()->get_body()->updateInertiaTensor();
    }

    // TODO ignore collision
}
