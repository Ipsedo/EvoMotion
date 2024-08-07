//
// Created by samuel on 05/04/24.
//

#include "./state.h"

#include <utility>
#include <glm/gtc/quaternion.hpp>

#include "../converter.h"

State::~State() = default;

ItemState::ItemState(
    Item item, const std::optional<Item> &root_item, const Item &floor, btDynamicsWorld *world)
    : root_item(root_item), floor_touched(false), is_not_root(root_item.has_value()),
      state_item(std::move(item)) {
    world->contactPairTest(state_item.get_body(), floor.get_body(), *this);
}

ItemState::ItemState(const Item &item, const Item &floor, btDynamicsWorld *world)
    : ItemState(item, {}, floor, world) {}

int ItemState::get_size() { return 3 + 3 * 4 + 1 + 7 * (3 + 3); }

torch::Tensor ItemState::get_point_state(const glm::vec3 point) const {
    glm::vec3 root_pos = is_not_root
                             ? root_item->model_matrix() * glm::vec4(glm::vec3(0.f), 1.f)
                             : glm::vec3(
                                 0.f);
    glm::vec3 pos = glm::vec3(state_item.model_matrix() * glm::vec4(point, 1.f)) - root_pos;

    btVector3 vel = state_item.get_body()->getVelocityInLocalPoint(
        state_item.get_body()->getCollisionShape()->getLocalScaling() *
        glm_to_bullet(point));

    return torch::tensor({pos.x, pos.y, pos.z, vel.x(), vel.y(), vel.z()});
}

torch::Tensor ItemState::get_state() {
    btScalar yaw, pitch, roll;
    state_item.get_body()->getWorldTransform().getRotation().getEulerZYX(yaw, pitch, roll);
    glm::quat quat = glm::quat_cast(state_item.model_matrix());

    const btVector3 center_lin_velocity = state_item.get_body()->getLinearVelocity();
    const btVector3 center_ang_velocity = state_item.get_body()->getAngularVelocity();

    const btVector3 force = state_item.get_body()->getTotalForce();
    const btVector3 torque = state_item.get_body()->getTotalTorque();

    float touched = floor_touched ? 1.f : -1.f;
    floor_touched = false;

    auto main_state = torch::tensor(
    {yaw / static_cast<float>(M_PI), pitch / static_cast<float>(M_PI),
     roll / static_cast<float>(M_PI), center_lin_velocity.x(), center_lin_velocity.y(),
     center_lin_velocity.z(), center_ang_velocity.x() / static_cast<float>(M_PI),
     center_ang_velocity.y() / static_cast<float>(M_PI),
     center_ang_velocity.z() / static_cast<float>(M_PI), force.x(), force.y(), force.z(),
     torque.x(), torque.y(), torque.z(), touched});

    return torch::cat(
        {main_state, get_point_state(glm::vec3(0, 0, 0)), get_point_state(glm::vec3(1, 0, 0)),
         get_point_state(glm::vec3(-1, 0, 0)), get_point_state(glm::vec3(0, 1, 0)),
         get_point_state(glm::vec3(0, -1, 0)), get_point_state(glm::vec3(0, 0, 1)),
         get_point_state(glm::vec3(0, 0, -1))},
        0);
}

btScalar ItemState::addSingleResult(
    btManifoldPoint &cp, const btCollisionObjectWrapper *colObj0Wrap, int partId0, int index0,
    const btCollisionObjectWrapper *colObj1Wrap, int partId1, int index1) {
    floor_touched = true;
    return 1.f;
}

ItemState::~ItemState() = default;

// Muscle
MuscleState::MuscleState(Muscle muscle) : slider_constraint(muscle.get_slider_constraint()) {
    auto [a, b] = muscle.get_p2p_constraints();
    p2p_a = a;
    p2p_b = b;

}

int MuscleState::get_size() { return 3 * 1; }

torch::Tensor MuscleState::get_state() {
    return torch::tensor(
    {
        slider_constraint->getAppliedImpulse(),
        p2p_a->getAppliedImpulse(),
        p2p_b->getAppliedImpulse()
    });
}

// Root

RootItemState::RootItemState(
    const Item &item, const Item &floor, btDynamicsWorld *world) : ItemState(
    item, floor,
    world) {}

int RootItemState::get_size() {
    return ItemState::get_size() - (3 + 3);// no origin point state for root
}

torch::Tensor RootItemState::get_point_state(glm::vec3 point) const {
    if (point != glm::vec3(0.f)) {
        glm::vec3 root_pos = bullet_to_glm(state_item.get_body()->getCenterOfMassPosition());
        glm::vec3 pos = glm::vec3(state_item.model_matrix() * glm::vec4(point, 1.f)) - root_pos;

        btVector3 vel = state_item.get_body()->getVelocityInLocalPoint(
            state_item.get_body()->getCollisionShape()->getLocalScaling() *
            glm_to_bullet(point));

        return torch::tensor({pos.x, pos.y, pos.z, vel.x(), vel.y(), vel.z()});
    }

    return torch::empty({0});
}