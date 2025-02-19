//
// Created by samuel on 12/02/25.
//

#include "./rotate.h"

#include <string>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>
#include <imgui.h>

RotateTools::RotateTools()
    : is_dragging(false), locked_direction(-1), prev_x_mouse(0.f), prev_y_mouse(0.f) {}

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

        if (const auto mouse_pos = ImGui::GetMousePos(); !is_dragging) {
            is_dragging = true;

            prev_x_mouse = mouse_pos.x;
            prev_y_mouse = mouse_pos.y;
        } else if (locked_direction == -1) {
            float dist_width = std::abs(prev_x_mouse - mouse_pos.x);
            float dist_height = std::abs(prev_y_mouse - mouse_pos.y);

            if (dist_width > dist_height) locked_direction = 0;
            else if (!ImGui::IsKeyDown(ImGuiKey_LeftShift)) locked_direction = 1;
            else locked_direction = 2;
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
        std::string third_dir;

        if (yaw_height < pitch_height && yaw_height < roll_height) vertical = "yaw";
        else if (pitch_height < yaw_height && pitch_height < roll_height) vertical = "pitch";
        else vertical = "roll";

        if (yaw_width < pitch_width && yaw_width < roll_width) horizontal = "yaw";
        else if (pitch_width < yaw_width && pitch_width < roll_width) horizontal = "pitch";
        else horizontal = "roll";

        if (vertical == "yaw" && horizontal == "pitch") third_dir = "roll";
        else if (vertical == "pitch" && horizontal == "yaw") third_dir = "roll";

        else if (vertical == "roll" && horizontal == "pitch") third_dir = "yaw";
        else if (vertical == "pitch" && horizontal == "roll") third_dir = "yaw";

        else if (vertical == "roll" && horizontal == "yaw") third_dir = "pitch";
        else if (vertical == "yaw" && horizontal == "roll") third_dir = "pitch";

        if (locked_direction == 0)
            angle = screen_coord.y < member_screen_pos.y ? screen_move_x : -screen_move_x;
        else if (locked_direction == 1)
            angle = screen_coord.x < member_screen_pos.x ? screen_move_y : -screen_move_y;
        else angle = screen_coord.x < member_screen_pos.x ? screen_move_y : -screen_move_y;

        prev_x_mouse = new_mouse_pos.x;
        prev_y_mouse = new_mouse_pos.y;

        std::string to_use;
        switch (locked_direction) {
            case 0: to_use = vertical; break;
            case 1: to_use = horizontal; break;
            case 2: to_use = third_dir; break;
            default: to_use = vertical; break;
        }

        const auto final_rot_axis = glm::vec3(axis_rot * rot_to_axis[to_use]);

        return glm::angleAxis(angle, final_rot_axis);
    } else {
        is_dragging = false;
        locked_direction = -1;
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
