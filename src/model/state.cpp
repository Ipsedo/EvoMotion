//
// Created by samuel on 05/04/24.
//

#include "./state.h"
#include "./converter.h"

ItemState::ItemState(const Item &item)
    : item(item), last_lin_velocity(0.f), last_ang_velocity(0.f) {}

int ItemState::get_size() { return 3 + /*3 * 6 +*/ 3 + 1 + 3 * 2 + 3 * 2; }

torch::Tensor ItemState::get_state() {
    glm::mat4 model_mat = item.model_matrix();

    glm::vec3 center_pos = model_mat * glm::vec4(glm::vec3(0), 1);

    /*glm::vec3 up_pos = model_mat * glm::vec4(glm::vec3(0, 1, 0), 1);
    glm::vec3 down_pos = model_mat * glm::vec4(glm::vec3(0, -1, 0), 1);

    glm::vec3 right_pos = model_mat * glm::vec4(glm::vec3(1, 0, 0), 1);
    glm::vec3 left_pos = model_mat * glm::vec4(glm::vec3(-1, 0, 0), 1);

    glm::vec3 back_pos = model_mat * glm::vec4(glm::vec3(0, 0, -1), 1);
    glm::vec3 front_pos = model_mat * glm::vec4(glm::vec3(0, 0, 1), 1);*/

    btQuaternion quat = item.get_body()->getOrientation();
    btVector3 axis = quat.getAxis();
    float angle = quat.getAngle();

    btVector3 center_lin_velocity = item.get_body()->getLinearVelocity();
    btVector3 center_ang_velocity = item.get_body()->getAngularVelocity();

    glm::vec3 lin_acc = bullet_to_glm(center_lin_velocity) - last_lin_velocity;
    glm::vec3 ang_acc = bullet_to_glm(center_ang_velocity) - last_ang_velocity;

    last_lin_velocity = bullet_to_glm(center_lin_velocity);
    last_ang_velocity = bullet_to_glm(center_ang_velocity);

    return torch::tensor(
        {center_pos.x, center_pos.y, center_pos.z,
         /*up_pos.x, up_pos.y, up_pos.z,
                             down_pos.x, down_pos.y, down_pos.z,
                             right_pos.x, right_pos.y, right_pos.z,
                             left_pos.x, left_pos.y, left_pos.z,
                             back_pos.x, back_pos.y, back_pos.z,
                             front_pos.x, front_pos.y, front_pos.z,*/
         axis.x(), axis.y(), axis.z(), angle, center_lin_velocity.x(), center_lin_velocity.y(),
         center_lin_velocity.z(), center_ang_velocity.x(), center_ang_velocity.y(),
         center_ang_velocity.z(), lin_acc.x, lin_acc.y, lin_acc.z, ang_acc.x, ang_acc.y,
         ang_acc.z});
}
