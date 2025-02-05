//
// Created by samuel on 31/01/25.
//

#include "./event.h"

#include <iostream>

#include <imgui.h>

MouseEvent::MouseEvent(float width, float height)
    : width(width), height(height), view_matrix(1.f), proj_matrix(1.f) {}

void MouseEvent::update(
    float new_width, float new_height, const glm::mat4 &new_view_matrix,
    const glm::mat4 &new_proj_matrix) {
    width = new_width;
    height = new_height;
    view_matrix = new_view_matrix;
    proj_matrix = new_proj_matrix;
}

std::optional<std::tuple<glm::vec3, glm::vec3>>
MouseEvent::get_scene_absolute_click_pos(float width_offset, float height_offset) {
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        float image_x = ImGui::GetMousePos().x - width_offset;
        float image_y = ImGui::GetMousePos().y - height_offset;

        glm::vec2 screen_coord(
            2.f * image_x / width - 1.f, -(2.f * image_y / height - 1.f) * height / width);

        glm::vec4 screen_coord_near = glm::vec4(screen_coord, -1.f, 1.f);
        glm::vec4 screen_coord_far = glm::vec4(screen_coord, 1.f, 1.f);

        glm::mat4 inv_proj_matrix = glm::inverse(proj_matrix);
        glm::vec4 cam_near = inv_proj_matrix * screen_coord_near;
        glm::vec4 cam_far = inv_proj_matrix * screen_coord_far;

        cam_near /= cam_near.w;
        cam_far /= cam_far.w;

        glm::mat4 inv_view_matrix = glm::inverse(view_matrix);
        glm::vec3 world_near = inv_view_matrix * cam_near;
        glm::vec3 world_far = inv_view_matrix * cam_far;

        return std::tuple(world_near, world_far);
    }
    return std::nullopt;
}
