//
// Created by samuel on 05/04/24.
//

#include "./state.h"

#include "../converter.h"

ItemState::ItemState(const Item &item)
    : item(item), last_lin_velocity(0.f), last_ang_velocity(0.f) {}

int ItemState::get_size() { return 3 + 3 + 3 * 2 + 3 * 2; }

torch::Tensor ItemState::get_state() {
    btVector3 center_pos = item.get_body()->getCenterOfMassPosition();

    btScalar yaw, pitch, roll;
    item.get_body()->getWorldTransform().getRotation().getEulerZYX(yaw, pitch, roll);

    btVector3 center_lin_velocity = item.get_body()->getLinearVelocity();
    btVector3 center_ang_velocity = item.get_body()->getAngularVelocity();

    btVector3 force = item.get_body()->getTotalForce();
    btVector3 torque = item.get_body()->getTotalTorque();

    return torch::tensor(
    {center_pos.x(), center_pos.y(), center_pos.z(),
     yaw, pitch, roll,
     center_lin_velocity.x(), center_lin_velocity.y(), center_lin_velocity.z(),
     center_ang_velocity.x(), center_ang_velocity.y(), center_ang_velocity.z(),
     force.x(), force.y(), force.z(), torque.x(), torque.y(), torque.z()});
}