//
// Created by samuel on 11/02/25.
//

#include "./robot_info.h"

#include <imgui.h>

#include "./utils.h"

RobotInfoWindow::RobotInfoWindow(const std::shared_ptr<RobotBuilderEnvironment> &builder_env)
    : ImGuiWindow("Robot information"), builder_env(builder_env) {}

void RobotInfoWindow::render_window_content(
    const std::shared_ptr<ItemFocusContext> &context,
    const std::shared_ptr<OpenGlWindow> &gl_window) {
    ImGui::Text("Robot selected : %s", builder_env->get_robot_name().c_str());

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    const auto root_name = builder_env->get_root_name();
    ImGui::Text("Root member : %s", root_name.has_value() ? root_name.value().c_str() : "No root");
    ImGui::Text("Members count : %i", builder_env->get_members_count());

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // robot name
    std::string robot_name = builder_env->get_robot_name();
    robot_name.resize(128);
    if (ImGui::InputText("Robot name", &robot_name[0], robot_name.size()))
        builder_env->set_robot_name(robot_name.c_str());

    // select root item
    int selected_item = 0;

    const std::vector<std::string> item_names = builder_env->get_member_names();
    for (int i = 0; i < item_names.size(); i++)
        if (root_name.has_value() && item_names[i] == root_name.value()) {
            selected_item = i;
            break;
        }

    if (ImGui::BeginCombo(
            "Select root member", root_name.has_value() ? root_name.value().c_str() : "No root")) {
        for (int i = 0; i < item_names.size(); i++)
            if (ImGui::Selectable(item_names[i].c_str(), i == selected_item)) {
                builder_env->set_root(item_names[i]);
                ImGui::SetItemDefaultFocus();
            }

        ImGui::EndCombo();
    }
}

void RobotInfoWindow::on_close(const std::shared_ptr<ItemFocusContext> &context) {}

void RobotInfoWindow::on_focus_change(
    bool new_focus, const std::shared_ptr<ItemFocusContext> &context) {}
