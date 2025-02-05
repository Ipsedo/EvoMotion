//
// Created by samuel on 28/01/25.
//

#include "./robot_tab.h"

#include <utility>

#include <imgui.h>

RobotTab::RobotTab(std::string name) : robot_name(std::move(name)) {}

void RobotTab::set_name(const std::string &new_name) { robot_name = new_name; }

std::string RobotTab::get_name() { return robot_name; }

void RobotTab::imgui_render() {
    if (ImGui::BeginTabItem(robot_name.c_str())) {
        ImGui::Text("Yo");

        ImGui::EndTabItem();
    }
}
