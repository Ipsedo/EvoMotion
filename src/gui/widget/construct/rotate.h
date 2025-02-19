//
// Created by samuel on 12/02/25.
//

#ifndef EVO_MOTION_ROTATE_H
#define EVO_MOTION_ROTATE_H

#include <functional>
#include <optional>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "../opengl_window.h"

class RotateTools {
public:
    RotateTools();

    std::optional<glm::quat> get_rot_delta(
        const std::shared_ptr<OpenGlWindow> &gl_window, const bool parent_cond,
        const std::tuple<float, float, float> &yaw_pitch_roll, const glm::vec3 member_absolute_pos);

private:
    bool is_dragging;
    float prev_x_mouse;
    float prev_y_mouse;

    static std::vector<glm::vec2> transform_circle_points(
        const std::tuple<float, float, float> &initial_yaw_pitch_roll, const glm::mat4 &mvp_matrix,
        int nb_points = 32);
    static float max_vertical(const std::vector<glm::vec2> &points);
    static float max_horizontal(const std::vector<glm::vec2> &points);
};

#endif//EVO_MOTION_ROTATE_H
