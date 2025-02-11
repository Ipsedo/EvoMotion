//
// Created by samuel on 11/02/25.
//

#include <imgui.h>

#include "./window.h"

RobotInfoWindow::RobotInfoWindow() : ImGuiWindow("Robot information") {}

void RobotInfoWindow::render_window_content(const std::shared_ptr<AppContext> &context) {
    ImGui::Spacing();

    ImGui::Text("Robot selected : %s", context->get_builder_env()->get_robot_name().c_str());
    ImGui::Text(
        "Member selected : %s",
        context->is_member_focused() ? context->get_focused_member().c_str() : "no member");

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Text("Root member : %s", context->get_builder_env()->get_root_name().c_str());
    ImGui::Text("Members count : %i", context->get_builder_env()->get_members_count());

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // robot name
    std::string robot_name = context->get_builder_env()->get_robot_name();
    robot_name.resize(128);
    if (ImGui::InputText("Robot name", &robot_name[0], robot_name.size()))
        context->get_builder_env()->set_robot_name(robot_name.c_str());

    // select root item
    int selected_item = 0;

    std::vector<std::string> item_names = context->get_builder_env()->get_member_names();
    const auto root_name = context->get_builder_env()->get_root_name();

    for (int i = 0; i < item_names.size(); i++)
        if (item_names[i] == root_name) {
            selected_item = i;
            break;
        }

    if (ImGui::Combo(
            "Select root item", &selected_item,
            [](void *user_ptr, const int idx, const char **out_text) {
                const auto &vec = *static_cast<std::vector<std::string> *>(user_ptr);
                if (idx < 0 || idx >= static_cast<int>(vec.size())) return false;
                *out_text = vec[idx].c_str();
                return true;
            },
            &item_names, static_cast<int>(item_names.size())))
        context->get_builder_env()->set_root(item_names[selected_item]);
}
