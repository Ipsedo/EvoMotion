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

    glm::mat4 model_mat = item.model_matrix_without_scale();
    glm::vec3 front_vector = model_mat * glm::vec4(0, 0, 1, 0);

    btVector3 center_lin_velocity = item.get_body()->getLinearVelocity();
    btVector3 center_ang_velocity = item.get_body()->getAngularVelocity();

    glm::vec3 lin_acc = bullet_to_glm(center_lin_velocity) - last_lin_velocity;
    glm::vec3 ang_acc = bullet_to_glm(center_ang_velocity) - last_ang_velocity;

    last_lin_velocity = bullet_to_glm(center_lin_velocity);
    last_ang_velocity = bullet_to_glm(center_ang_velocity);

    return torch::tensor(
        {center_pos.x(), center_pos.y(), center_pos.z(), front_vector.x, front_vector.y, front_vector.z,
         center_lin_velocity.x(), center_lin_velocity.y(), center_lin_velocity.z(),
         center_ang_velocity.x(), center_ang_velocity.y(), center_ang_velocity.z(), lin_acc.x,
         lin_acc.y, lin_acc.z, ang_acc.x, ang_acc.y, ang_acc.z});
}
