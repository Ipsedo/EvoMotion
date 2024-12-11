//
// Created by samuel on 17/12/22.
//

#include <utility>

#include <evo_motion_view/camera.h>

// Abstract Camera

Camera::~Camera() = default;

// Static Camera

StaticCamera::StaticCamera(const glm::vec3 pos, const glm::vec3 look, const glm::vec3 up)
    : pos_vec(pos), look_vec(look), up_vec(up) {}

glm::vec3 StaticCamera::pos() { return pos_vec; }

glm::vec3 StaticCamera::look() { return look_vec; }

glm::vec3 StaticCamera::up() { return up_vec; }

void StaticCamera::step(float delta) {}

void StaticCamera::reset() {}

FollowCamera::FollowCamera(std::function<glm::vec3()> get_object_center)
    : get_object_center(std::move(get_object_center)), object_smooth_pos(0.f), distance(5.f),
      up_angle(static_cast<float>(M_PI) / 2.f), side_angle(0.f), pos_vec(1.f), look_vec(0.f),
      up_vec(1), factor(2.f) {}

glm::vec3 FollowCamera::pos() { return pos_vec; }

glm::vec3 FollowCamera::look() { return look_vec; }

glm::vec3 FollowCamera::up() { return up_vec; }

void FollowCamera::step(float delta) {

    object_smooth_pos += (get_object_center() - object_smooth_pos) * factor * delta;

    pos_vec =
        object_smooth_pos
        + glm::vec3(std::cos(side_angle) * distance, std::sin(up_angle), std::sin(side_angle) * distance);
    look_vec = object_smooth_pos;
    up_vec = glm::vec3(0, 1.f, 0);
}

void FollowCamera::reset() { object_smooth_pos = get_object_center(); }
