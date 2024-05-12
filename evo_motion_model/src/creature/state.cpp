//
// Created by samuel on 05/04/24.
//

#include <utility>

#include "./state.h"

#include "../converter.h"

State::~State() = default;

ItemState::ItemState(Item item, const Item &floor, btDynamicsWorld *world)
    : state_item(std::move(item)), floor_touched(false) {
    world->contactPairTest(state_item.get_body(), floor.get_body(), *this);
}

int ItemState::get_size() { return 3 + 3 + 3 * 4 + 3 + 1 + 6 * (3 + 3); }

torch::Tensor ItemState::get_point_state(const glm::vec3 point) const {
    const auto model_mat = state_item.model_matrix();

    glm::vec3 pos = model_mat * glm::vec4(point, 1.f);
    const btVector3 vel = state_item.get_body()->getVelocityInLocalPoint(glm_to_bullet(point));

    return torch::tensor({
        pos.x, pos.y, pos.z,
        vel.x(), vel.y(), vel.z()
    });
}

torch::Tensor ItemState::get_state() {
    const btVector3 center_pos = state_item.get_body()->getCenterOfMassPosition();

    btScalar yaw, pitch, roll;
    state_item.get_body()->getWorldTransform().getRotation().getEulerZYX(yaw, pitch, roll);

    const btVector3 up_axis = state_item.get_body()->getWorldTransform() * btVector4(0, 1, 0, 0);

    const btVector3 center_lin_velocity = state_item.get_body()->getLinearVelocity();
    const btVector3 center_ang_velocity = state_item.get_body()->getAngularVelocity();

    const btVector3 force = state_item.get_body()->getTotalForce();
    const btVector3 torque = state_item.get_body()->getTotalTorque();

    float touched = floor_touched ? 1.f : 0.f;
    floor_touched = false;

    auto main_state = torch::tensor(
    {center_pos.x(), center_pos.y(), center_pos.z(),
     yaw / static_cast<float>(M_PI), pitch / static_cast<float>(M_PI), roll / static_cast<float>(M_PI),
     center_lin_velocity.x(), center_lin_velocity.y(), center_lin_velocity.z(),
     center_ang_velocity.x(), center_ang_velocity.y(), center_ang_velocity.z(),
     force.x(), force.y(), force.z(), torque.x(), torque.y(), torque.z(),
        up_axis.x(), up_axis.y(), up_axis.z(), touched});

    return torch::cat({
        main_state,
        get_point_state(glm::vec3(1, 0, 0)),
        get_point_state(glm::vec3(-1, 0, 0)),
        get_point_state(glm::vec3(0, 1, 0)),
        get_point_state(glm::vec3(0, -1, 0)),
        get_point_state(glm::vec3(0, 0, 1)),
        get_point_state(glm::vec3(0, 0, -1))
    }, 0);
}

btScalar ItemState::addSingleResult(
    btManifoldPoint &cp, const btCollisionObjectWrapper *colObj0Wrap, int partId0, int index0,
    const btCollisionObjectWrapper *colObj1Wrap, int partId1, int index1) {
    floor_touched = true;
    return 1.f;
}

ItemState::~ItemState() = default;
