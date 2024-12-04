//
// Created by samuel on 05/04/24.
//

#include "./state.h"

#include <utility>

#include <glm/gtc/quaternion.hpp>

#include "../converter.h"

State::~State() = default;

// Base class for proprioception
ItemProprioceptionState::ItemProprioceptionState(
    const Item &item, const Item &floor, btDynamicsWorld *world)
    : state_item(item), floor_touched(false), last_ang_vel(0, 0, 0), last_lin_vel(0, 0, 0) {
    world->contactPairTest(state_item.get_body(), floor.get_body(), *this);
}

int ItemProprioceptionState::get_size() { return 3 + 3 * 4 + 1 /* + 6 * 3*/; }

torch::Tensor ItemProprioceptionState::get_state(torch::Device device) {
    btScalar yaw, pitch, roll;
    state_item.get_body()->getWorldTransform().getRotation().getEulerZYX(yaw, pitch, roll);

    const btVector3 center_lin_velocity = state_item.get_body()->getLinearVelocity();
    const btVector3 center_ang_velocity = state_item.get_body()->getAngularVelocity();

    const btVector3 center_lin_acceleration = last_lin_vel - center_lin_velocity;
    const btVector3 center_ang_acceleration = last_ang_vel - center_ang_velocity;

    last_lin_vel = center_lin_velocity;
    last_ang_vel = center_ang_velocity;

    /*const btVector3 force = state_item.get_body()->getTotalForce();
    const btVector3 torque = state_item.get_body()->getTotalTorque();*/

    float touched = floor_touched ? 1.f : 0.f;
    floor_touched = false;

    return torch::tensor(
        {yaw / static_cast<float>(M_PI), pitch / static_cast<float>(M_PI),
         roll / static_cast<float>(M_PI), center_lin_velocity.x(), center_lin_velocity.y(),
         center_lin_velocity.z(), center_ang_velocity.x() / static_cast<float>(M_PI),
         center_ang_velocity.y() / static_cast<float>(M_PI),
         center_ang_velocity.z() / static_cast<float>(M_PI), center_lin_acceleration.x(),
         center_lin_acceleration.y(), center_lin_acceleration.z(),
         center_ang_acceleration.x() / static_cast<float>(M_PI),
         center_ang_acceleration.y() / static_cast<float>(M_PI),
         center_ang_acceleration.z() / static_cast<float>(M_PI), touched},
        at::TensorOptions().device(device));

    /*return torch::cat(
    {main_state, get_point_state(glm::vec3(1, 0, 0)), get_point_state(glm::vec3(-1, 0, 0)),
         get_point_state(glm::vec3(0, 1, 0)), get_point_state(glm::vec3(0, -1, 0)),
         get_point_state(glm::vec3(0, 0, 1)), get_point_state(glm::vec3(0, 0, -1))});*/
}

torch::Tensor ItemProprioceptionState::get_point_state(const glm::vec3 point) const {
    const glm::vec3 center_pos = state_item.model_matrix() * glm::vec4(glm::vec3(0.f), 1.f);
    glm::vec3 pos = glm::vec3(state_item.model_matrix() * glm::vec4(point, 1.f)) - center_pos;

    const btVector3 vel = state_item.get_body()->getVelocityInLocalPoint(
        state_item.get_body()->getCollisionShape()->getLocalScaling() * glm_to_bullet(point));

    return torch::tensor({pos.x, pos.y, pos.z, vel.x(), vel.y(), vel.z()});
}

btScalar ItemProprioceptionState::addSingleResult(
    btManifoldPoint &cp, const btCollisionObjectWrapper *colObj0Wrap, int partId0, int index0,
    const btCollisionObjectWrapper *colObj1Wrap, int partId1, int index1) {
    floor_touched = true;
    return 1.f;
}

// Member class

MemberState::MemberState(
    const Item &item, const Item &root_item, const Item &floor, btDynamicsWorld *world)
    : ItemProprioceptionState(item, floor, world), root_item(root_item) {}

int MemberState::get_size() { return ItemProprioceptionState::get_size() + 3; }

torch::Tensor MemberState::get_state(torch::Device device) {
    glm::vec4 center_point(0.f, 0.f, 0.f, 1.f);
    glm::vec3 center_pos =
        state_item.model_matrix() * center_point - root_item.model_matrix() * center_point;
    return torch::cat(
        {ItemProprioceptionState::get_state(device),
         torch::tensor(
             {center_pos.x, center_pos.y, center_pos.z}, at::TensorOptions().device(device))});
}

// Root Member class

RootMemberState::RootMemberState(const Item &item, const Item &floor, btDynamicsWorld *world)
    : ItemProprioceptionState(item, floor, world) {}

int RootMemberState::get_size() { return ItemProprioceptionState::get_size() + 3; }

torch::Tensor RootMemberState::get_state(torch::Device device) {
    const auto center_pos = state_item.get_body()->getCenterOfMassPosition();
    return torch::cat(
        {ItemProprioceptionState::get_state(device),
         torch::tensor(
             {log(center_pos.norm() + 1.f), center_pos.y(), atan2(center_pos.z(), center_pos.x())},
             at::TensorOptions().device(device))});
}

// Muscle
MuscleState::MuscleState(Muscle muscle) : slider_constraint(muscle.get_slider_constraint()) {
    auto [a, b] = muscle.get_p2p_constraints();
    p2p_a = a;
    p2p_b = b;
}

int MuscleState::get_size() { return 4; }

torch::Tensor MuscleState::get_state(torch::Device device) {
    return torch::tensor(
        {slider_constraint->getLinearPos(), slider_constraint->getAppliedImpulse(),
         p2p_a->getAppliedImpulse(), p2p_b->getAppliedImpulse()},
        at::TensorOptions().device(device));
}
