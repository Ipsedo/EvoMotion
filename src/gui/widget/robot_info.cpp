//
// Created by samuel on 11/02/25.
//

#include "./robot_info.h"

#include <imgui.h>

#include "./utils.h"

RobotInfoWindow::RobotInfoWindow(const std::shared_ptr<RobotBuilderEnvironment> &builder_env)
    : ImGuiWindow("Robot information"), builder_env(builder_env) {}

void RobotInfoWindow::render_window_content(const std::shared_ptr<ItemFocusContext> &context) {
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
    if (input_text("Robot name", &robot_name[0], robot_name.size(), 16))
        builder_env->set_robot_name(robot_name.c_str());

    // select root item
    int selected_item = 0;

    const std::vector<std::string> item_names = builder_env->get_member_names();
    for (int i = 0; i < item_names.size(); i++)
        if (root_name.has_value() && item_names[i] == root_name.value()) {
            selected_item = i;
            break;
        }

    const ImVec2 root_member_title_size = ImGui::CalcTextSize("Select root member");
    const ImVec2 default_root_size = ImGui::CalcTextSize("No root");
    ImGui::SetNextItemWidth(
        root_member_title_size.x + default_root_size.x + ImGui::GetStyle().FramePadding.x * 4.f);
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
