//
// Created by samuel on 11/02/25.
//

#include <imgui.h>

#include "../window.h"

NewConstraintWindow::NewConstraintWindow()
    : ImGuiWindow("New constraint"), constraint_name(), parent_name(std::nullopt),
      child_name(std::nullopt), absolute_position(0.f) {}

void NewConstraintWindow::render_window_content(const std::shared_ptr<AppContext> &context) {

    const auto member_names = context->builder_env.get()->get_member_names();

    ImGui::Columns(2, nullptr, false);

    // constraint name
    constraint_name.resize(128);
    ImGui::InputText("Name", &constraint_name[0], constraint_name.size());
    constraint_name = constraint_name.c_str();

    ImGui::Spacing();

    // parent
    if (ImGui::BeginCombo(
            "Parent", parent_name.has_value() ? parent_name.value().c_str() : "No parent")) {
        for (int i = 0; i < member_names.size(); i++)
            if ((!child_name.has_value() || child_name.value() != member_names[i])
                && ImGui::Selectable(
                    member_names[i].c_str(),
                    parent_name.has_value() && parent_name.value() == member_names[i])) {

                parent_name = member_names[i];
                context->constraint_parent.set(parent_name.value());
            }
        ImGui::EndCombo();
    }

    ImGui::Spacing();

    // child
    if (ImGui::BeginCombo(
            "Child", child_name.has_value() ? child_name.value().c_str() : "No child")) {
        for (int i = 0; i < member_names.size(); i++)
            if ((!parent_name.has_value() || parent_name.value() != member_names[i])
                && ImGui::Selectable(
                    member_names[i].c_str(),
                    child_name.has_value() && child_name.value() == member_names[i])) {
                child_name = member_names[i];
                context->constraint_child.set(child_name.value());
            }

        ImGui::EndCombo();
    }

    ImGui::Spacing();
}
