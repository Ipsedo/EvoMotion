//
// Created by samuel on 05/04/24.
//

#include "./state.h"

#include "../converter.h"

ItemState::ItemState(const Item &item)
    : item(item), last_lin_velocity(0.f), last_ang_velocity(0.f) {}

int ItemState::get_size() { return 3 + 3 + 3 * 2 + 3 * 2 + 6 * 3; }

torch::Tensor ItemState::get_state() {
    btVector3 center_pos = item.get_body()->getCenterOfMassPosition() / 1000;

    glm::mat4 model_mat = item.model_matrix_without_scale();
    glm::vec3 front_vector = model_mat * glm::vec4(0, 0, 1, 0);

    glm::mat4 model_mat_scale = item.model_matrix();

    glm::vec3 z_1 = glm::vec3(model_mat_scale * glm::vec4(0, 0, 1, 1));
    glm::vec3 z_2 = glm::vec3(model_mat_scale * glm::vec4(0, 0, -1, 1));

    glm::vec3 x_1 = glm::vec3(model_mat_scale * glm::vec4(1, 0, 0, 1));
    glm::vec3 x_2 = glm::vec3(model_mat_scale * glm::vec4(-1, 0, 0, 1));

    glm::vec3 y_1 = glm::vec3(model_mat_scale * glm::vec4(0, 1, 0, 1));
    glm::vec3 y_2 = glm::vec3(model_mat_scale * glm::vec4(0, -1, 0, 1));

    btVector3 center_lin_velocity = item.get_body()->getLinearVelocity();
    btVector3 center_ang_velocity = item.get_body()->getAngularVelocity();

    auto lin_vel = bullet_to_glm(center_lin_velocity);
    auto ang_vel = bullet_to_glm(center_ang_velocity);

    glm::vec3 lin_acc = lin_vel - last_lin_velocity;
    glm::vec3 ang_acc = ang_vel - last_ang_velocity;

    last_lin_velocity = lin_vel;
    last_ang_velocity = ang_vel;

    return torch::tensor(
    {center_pos.x(),
     center_pos.y(),
     center_pos.z(),
     x_1.x,
     x_1.y,
     x_1.z,
     x_2.x,
     x_2.y,
     x_2.z,
     y_1.x,
     y_1.y,
     y_1.z,
     y_2.x,
     y_2.y,
     y_2.z,
     z_1.x,
     z_1.y,
     z_1.z,
     z_2.x,
     z_2.y,
     z_2.y,
     front_vector.x,
     front_vector.y,
     front_vector.z,
     center_lin_velocity.x(),
     center_lin_velocity.y(),
     center_lin_velocity.z(),
     center_ang_velocity.x(),
     center_ang_velocity.y(),
     center_ang_velocity.z(),
     lin_acc.x,
     lin_acc.y,
     lin_acc.z,
     ang_acc.x,
     ang_acc.y,
     ang_acc.z});
}

// Skeleton Item state

SkeletonItemState::SkeletonItemState(const Item &root, const Item &item)
    : root(root), item(item), last_lin_velocity(0.f), last_ang_velocity(0.f) {}

int SkeletonItemState::get_size() { return 3 + 3 + 3 * 2 + 3 * 2 + 6 * 3; }

torch::Tensor SkeletonItemState::get_state() {
    glm::vec3 root_pos = bullet_to_glm(item.get_body()->getCenterOfMassPosition());
    glm::vec3 center_pos = bullet_to_glm(item.get_body()->getCenterOfMassPosition()) - root_pos;

    glm::mat4 model_mat = item.model_matrix_without_scale();
    glm::vec3 front_vector = model_mat * glm::vec4(0, 0, 1, 0);

    glm::mat4 model_mat_scale = item.model_matrix();

    glm::vec3 z_1 = glm::vec3(model_mat_scale * glm::vec4(0, 0, 1, 1)) - root_pos;
    glm::vec3 z_2 = glm::vec3(model_mat_scale * glm::vec4(0, 0, -1, 1)) - root_pos;

    glm::vec3 x_1 = glm::vec3(model_mat_scale * glm::vec4(1, 0, 0, 1)) - root_pos;
    glm::vec3 x_2 = glm::vec3(model_mat_scale * glm::vec4(-1, 0, 0, 1)) - root_pos;

    glm::vec3 y_1 = glm::vec3(model_mat_scale * glm::vec4(0, 1, 0, 1)) - root_pos;
    glm::vec3 y_2 = glm::vec3(model_mat_scale * glm::vec4(0, -1, 0, 1)) - root_pos;

    btVector3 center_lin_velocity = item.get_body()->getLinearVelocity();
    btVector3 center_ang_velocity = item.get_body()->getAngularVelocity();

    auto lin_vel = bullet_to_glm(center_lin_velocity);
    auto ang_vel = bullet_to_glm(center_ang_velocity);

    glm::vec3 lin_acc = lin_vel - last_lin_velocity;
    glm::vec3 ang_acc = ang_vel - last_ang_velocity;

    last_lin_velocity = lin_vel;
    last_ang_velocity = ang_vel;

    return torch::tensor(
    {center_pos.x,
     center_pos.y,
     center_pos.z,
     x_1.x,
     x_1.y,
     x_1.z,
     x_2.x,
     x_2.y,
     x_2.z,
     y_1.x,
     y_1.y,
     y_1.z,
     y_2.x,
     y_2.y,
     y_2.z,
     z_1.x,
     z_1.y,
     z_1.z,
     z_2.x,
     z_2.y,
     z_2.y,
     front_vector.x,
     front_vector.y,
     front_vector.z,
     center_lin_velocity.x(),
     center_lin_velocity.y(),
     center_lin_velocity.z(),
     center_ang_velocity.x(),
     center_ang_velocity.y(),
     center_ang_velocity.z(),
     lin_acc.x,
     lin_acc.y,
     lin_acc.z,
     ang_acc.x,
     ang_acc.y,
     ang_acc.z});
}
