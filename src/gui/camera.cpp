//
// Created by samuel on 29/01/25.
//

#include "./camera.h"

#include <iostream>

#include <imgui.h>

ImGuiCamera::ImGuiCamera()
    : in_action(false), last_x(0), last_y(0), factor(1e-2f), vertical_angle(M_PI / 4.f),
      horizontal_angle(0.f), distance(4.f), get_item_pos(std::nullopt) {}

void ImGuiCamera::set_focus(const std::function<glm::vec3()> &new_get_item_pos) {
    get_item_pos = new_get_item_pos;
}
void ImGuiCamera::release_focus() { get_item_pos = std::nullopt; }

void ImGuiCamera::update() {
    if (ImGui::IsMouseDragging(ImGuiMouseButton_Right)) {
        ImVec2 mouse_pos = ImGui::GetMousePos();
        if (!in_action) {
            last_x = mouse_pos.x;
            last_y = mouse_pos.y;
            in_action = true;
        }

        int delta_x = mouse_pos.x - last_x;
        int delta_y = mouse_pos.y - last_y;

        horizontal_angle += static_cast<float>(delta_x) * factor;

        vertical_angle -= static_cast<float>(delta_y) * factor;
        vertical_angle = std::max(static_cast<float>(M_PI) / 6.f, vertical_angle);
        vertical_angle = std::min(2.f * static_cast<float>(M_PI) / 3.f, vertical_angle);

        last_x = mouse_pos.x;
        last_y = mouse_pos.y;
    } else {
        in_action = false;
    }

    distance += ImGui::GetIO().MouseWheel * 1e-1f;
    distance = std::max(1.f, distance);
    distance = std::min(20.f, distance);
}

glm::vec3 ImGuiCamera::pos() {

    const glm::vec3 relative_pos(
        std::sin(vertical_angle) * std::cos(horizontal_angle), std::cos(vertical_angle),
        std::sin(vertical_angle) * std::sin(horizontal_angle));
    const auto item_pos = look();

    return relative_pos * distance + item_pos;
}

glm::vec3 ImGuiCamera::look() {
    if (get_item_pos.has_value()) return get_item_pos.value()();
    return glm::vec3(0.f);
}

glm::vec3 ImGuiCamera::up() { return glm::vec3(0, 1, 0); }

void ImGuiCamera::step(float delta) {}

void ImGuiCamera::reset() {
    vertical_angle = M_PI / 4.f;
    horizontal_angle = 0.f;
    distance = 4.f;
}
