//
// Created by samuel on 17/02/25.
//

#include "./translate.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>
#include <imgui.h>

TranslateTools::TranslateTools() : is_dragging(false), prev_x_mouse(0.f), prev_y_mouse(0.f) {}

std::optional<glm::vec3> TranslateTools::get_pos_delta(
    const bool parent_cond, const std::tuple<float, float, float> &yaw_pitch_roll,
    const glm::mat4 &projection_matrix, const glm::mat4 &view_matrix) {
    if (parent_cond && ImGui::IsMouseDragging(ImGuiMouseButton_Left) && !ImGui::IsWindowHovered()) {

        const auto [yaw, pitch, roll] = yaw_pitch_roll;
        const auto axis_rot = glm::yawPitchRoll(
            static_cast<float>(M_PI) * yaw / 180.f, static_cast<float>(M_PI) * pitch / 180.f,
            static_cast<float>(M_PI) * roll / 180.f);

        if (!is_dragging) {
            is_dragging = true;
            const auto mouse_pos = ImGui::GetMousePos();
            prev_x_mouse = mouse_pos.x;
            prev_y_mouse = mouse_pos.y;
        }

        glm::mat4 mvp_matrix = projection_matrix * view_matrix * axis_rot;

        glm::vec3 proj_x = mvp_matrix * glm::vec4(1, 0, 0, 0);
        glm::vec3 proj_y = mvp_matrix * glm::vec4(0, 1, 0, 0);
        glm::vec3 proj_z = mvp_matrix * glm::vec4(0, 0, 1, 0);

        const auto new_mouse_pos = ImGui::GetMousePos();
        float screen_move_x = (new_mouse_pos.x - prev_x_mouse) / 100.f,
              screen_move_y = (new_mouse_pos.y - prev_y_mouse) / 100.f;

        float mouse_threshold = 1e-2f;

        if (std::abs(screen_move_x) < mouse_threshold) screen_move_x = 0.f;
        else screen_move_x += screen_move_x < 0 ? mouse_threshold : -mouse_threshold;
        if (std::abs(screen_move_y) < mouse_threshold) screen_move_y = 0.f;
        else screen_move_y += screen_move_y < 0 ? mouse_threshold : -mouse_threshold;

        float x_x = std::abs(proj_x.x), x_y = std::abs(proj_x.y);
        float y_x = std::abs(proj_y.x), y_y = std::abs(proj_y.y);
        float z_x = std::abs(proj_z.x), z_y = std::abs(proj_z.y);

        glm::vec3 new_move(0.f);

        std::string horizontal_axis;
        std::string vertical_axis;

        // horizontal
        if (x_x >= y_x && x_x >= z_x) {
            horizontal_axis = "x";
            new_move.x = proj_x.x > 0 ? screen_move_x : -screen_move_x;
        } else if (y_x >= x_x && y_x >= z_x) {
            horizontal_axis = "y";
            new_move.y = proj_y.x > 0 ? screen_move_x : -screen_move_x;
        } else {
            horizontal_axis = "z";
            new_move.z = proj_z.x > 0 ? screen_move_x : -screen_move_x;
        }

        // vertical
        if (x_y >= y_y && x_y >= z_y) vertical_axis = "x";
        else if (y_y >= x_y && y_y >= z_y) vertical_axis = "y";
        else vertical_axis = "z";

        if (!ImGui::IsKeyDown(ImGuiKey_LeftShift)) {
            if (vertical_axis == "x") new_move.x = proj_x.y > 0 ? -screen_move_y : screen_move_y;
            else if (vertical_axis == "y")
                new_move.y = proj_y.y > 0 ? -screen_move_y : screen_move_y;
            else if (vertical_axis == "z")
                new_move.z = proj_z.y > 0 ? -screen_move_y : screen_move_y;
        } else {
            if (vertical_axis == "x" && horizontal_axis == "y")
                new_move.z = proj_x.y > 0 ? screen_move_y : -screen_move_y;
            else if (vertical_axis == "x" && horizontal_axis == "z")
                new_move.y = proj_x.y > 0 ? screen_move_y : -screen_move_y;

            else if (vertical_axis == "y" && horizontal_axis == "x")
                new_move.z = proj_y.y > 0 ? screen_move_y : -screen_move_y;
            else if (vertical_axis == "y" && horizontal_axis == "z")
                new_move.x = proj_y.y > 0 ? screen_move_y : -screen_move_y;

            else if (vertical_axis == "z" && horizontal_axis == "x")
                new_move.y = proj_z.y > 0 ? screen_move_y : -screen_move_y;
            else if (vertical_axis == "z" && horizontal_axis == "x")
                new_move.y = proj_z.y > 0 ? screen_move_y : -screen_move_y;
        }

        prev_x_mouse = new_mouse_pos.x;
        prev_y_mouse = new_mouse_pos.y;

        return glm::vec3(axis_rot * glm::vec4(new_move, 0.f));
    } else {
        is_dragging = false;
        return std::nullopt;
    }
}
