//
// Created by samuel on 17/12/22.
//

#include <evo_motion_view/camera.h>

// Abstract Camera

Camera::~Camera() = default;

// Static Camera

StaticCamera::StaticCamera(const glm::vec3 pos, const glm::vec3 look, const glm::vec3 up)
    : pos_vec(pos), look_vec(look), up_vec(up) {}

glm::vec3 StaticCamera::pos() { return pos_vec; }

glm::vec3 StaticCamera::look() { return look_vec; }

glm::vec3 StaticCamera::up() { return up_vec; }