//
// Created by samuel on 12/02/25.
//

#include "./rotate.h"

#include <string>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>
#include <imgui.h>

RotateTools::RotateTools() : is_dragging(false), prev_x_mouse(0.f), prev_y_mouse(0.f) {}

std::optional<glm::quat> RotateTools::get_rot_delta(
    const std::shared_ptr<OpenGlWindow> &gl_window, const bool parent_cond,
    const std::tuple<float, float, float> &yaw_pitch_roll, const glm::vec3 member_absolute_pos) {
    if (parent_cond && ImGui::IsMouseDragging(ImGuiMouseButton_Left) && !ImGui::IsWindowHovered()) {

        const auto [yaw, pitch, roll] = yaw_pitch_roll;
        const auto axis_rot = glm::yawPitchRoll(
            static_cast<float>(M_PI) * yaw / 180.f, static_cast<float>(M_PI) * pitch / 180.f,
            static_cast<float>(M_PI) * roll / 180.f);

        std::map<std::string, glm::vec4> rot_to_axis = {
            {"yaw", glm::vec4(0, 1, 0, 0)},
            {"pitch", glm::vec4(1, 0, 0, 0)},
            {"roll", glm::vec4(0, 0, 1, 0)}};

        if (!is_dragging) {
            is_dragging = true;
            const auto mouse_pos = ImGui::GetMousePos();
            prev_x_mouse = mouse_pos.x;
            prev_y_mouse = mouse_pos.y;
        }

        const auto new_mouse_pos = ImGui::GetMousePos();
        float screen_move_x = (new_mouse_pos.x - prev_x_mouse) / 100.f,
              screen_move_y = (new_mouse_pos.y - prev_y_mouse) / 100.f;

        const auto [width_offset, height_offset] = gl_window->get_window_start_pos();
        const auto [width, height] = gl_window->get_width_height();
        glm::vec2 screen_coord(
            2.f * (new_mouse_pos.x - width_offset) / width - 1.f,
            -(2.f * (new_mouse_pos.y - height_offset) / height - 1.f));

        glm::mat4 mvp_matrix = gl_window->get_projection_matrix() * gl_window->get_view_matrix()
                               * glm::translate(glm::mat4(1), member_absolute_pos) * axis_rot;

        const auto member_projected_pos = gl_window->get_projection_matrix()
                                          * gl_window->get_view_matrix()
                                          * glm::vec4(member_absolute_pos, 1.f);
        const auto member_screen_pos = glm::vec2(member_projected_pos / member_projected_pos.w);

        const auto circle_points_yaw = transform_circle_points({0.f, 0.f, 0.f}, mvp_matrix);
        const auto circle_points_pitch = transform_circle_points({0.f, 0.f, 90.f}, mvp_matrix);
        const auto circle_points_roll = transform_circle_points({90.f, 0.f, 90.f}, mvp_matrix);

        const auto yaw_height = max_vertical(circle_points_yaw);
        const auto pitch_height = max_vertical(circle_points_pitch);
        const auto roll_height = max_vertical(circle_points_roll);

        const auto yaw_width = max_horizontal(circle_points_yaw);
        const auto pitch_width = max_horizontal(circle_points_pitch);
        const auto roll_width = max_horizontal(circle_points_roll);

        float angle = 0.f;

        std::string vertical;
        std::string horizontal;

        if (yaw_height < pitch_height && yaw_height < roll_height) vertical = "yaw";
        else if (pitch_height < yaw_height && pitch_height < roll_height) vertical = "pitch";
        else vertical = "roll";

        angle = screen_coord.y < member_screen_pos.y ? screen_move_x : -screen_move_x;

        prev_x_mouse = new_mouse_pos.x;
        prev_y_mouse = new_mouse_pos.y;

        const auto final_rot_axis = glm::vec3(axis_rot * rot_to_axis[vertical]);

        return glm::angleAxis(angle, final_rot_axis);
    } else {
        is_dragging = false;
        return std::nullopt;
    }
}

std::vector<glm::vec2> RotateTools::transform_circle_points(
    const std::tuple<float, float, float> &initial_yaw_pitch_roll, const glm::mat4 &mvp_matrix,
    const int nb_points) {
    std::vector<glm::vec2> points;

    const auto [ini_yaw, init_pitch, ini_roll] = initial_yaw_pitch_roll;
    glm::mat4 ini_rot = glm::yawPitchRoll(ini_yaw, init_pitch, ini_roll);

    for (int i = 0; i < nb_points; i++) {
        const auto point =
            mvp_matrix * ini_rot
            * glm::vec4(
                std::cos(2 * M_PI * i / nb_points), 0, std::sin(2 * M_PI * i / nb_points), 1.f);
        points.push_back(glm::vec2(point) / point.w);
    }

    return points;
}

float RotateTools::max_vertical(const std::vector<glm::vec2> &points) {
    float min = std::numeric_limits<float>::max();
    float max = -std::numeric_limits<float>::max();

    for (const auto &p: points) {
        min = std::min(p.y, min);
        max = std::max(p.y, max);
    }
    return std::abs(max - min);
}

float RotateTools::max_horizontal(const std::vector<glm::vec2> &points) {
    float min = std::numeric_limits<float>::max();
    float max = -std::numeric_limits<float>::max();

    for (const auto &p: points) {
        min = std::min(p.x, min);
        max = std::max(p.x, max);
    }
    return std::abs(max - min);
}
