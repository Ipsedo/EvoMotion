//
// Created by samuel on 11/02/25.
//

#include <imgui.h>

#include "./window.h"

RobotInfoWindow::RobotInfoWindow() : ImGuiWindow("Robot information") {}

void RobotInfoWindow::render_window_content(const std::shared_ptr<AppContext> &context) {
    ImGui::Spacing();

    if (context->builder_env.is_set()) {

        ImGui::Text("Robot selected : %s", context->builder_env.get()->get_robot_name().c_str());
        ImGui::Text(
            "Member selected : %s",
            context->focused_member.is_set() ? context->focused_member.get().c_str() : "no member");

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        const auto root_name = context->builder_env.get()->get_root_name();
        ImGui::Text(
            "Root member : %s", root_name.has_value() ? root_name.value().c_str() : "No root");
        ImGui::Text("Members count : %i", context->builder_env.get()->get_members_count());

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // robot name
        std::string robot_name = context->builder_env.get()->get_robot_name();
        robot_name.resize(128);
        if (ImGui::InputText("Robot name", &robot_name[0], robot_name.size()))
            context->builder_env.get()->set_robot_name(robot_name.c_str());

        // select root item
        int selected_item = 0;

        std::vector<std::string> item_names = context->builder_env.get()->get_member_names();
        for (int i = 0; i < item_names.size(); i++)
            if (root_name.has_value() && item_names[i] == root_name.value()) {
                selected_item = i;
                break;
            }

        if (ImGui::BeginCombo(
                "Select root member",
                root_name.has_value() ? root_name.value().c_str() : "No root")) {
            for (int i = 0; i < item_names.size(); i++)
                if (ImGui::Selectable(item_names[i].c_str(), i == selected_item)) {
                    context->builder_env.get()->set_root(item_names[i]);
                    ImGui::SetItemDefaultFocus();
                }

            ImGui::EndCombo();
        }
    }
}
