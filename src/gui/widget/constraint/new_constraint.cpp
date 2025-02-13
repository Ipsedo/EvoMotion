//
// Created by samuel on 11/02/25.
//

#include "./new_constraint.h"

#include <imgui.h>

NewConstraintWindow::NewConstraintWindow(
    const std::shared_ptr<RobotBuilderEnvironment> &builder_env)
    : ImGuiWindow("New constraint"), builder_env(builder_env), constraint_name(),
      parent_name(std::nullopt), child_name(std::nullopt), absolute_position(0.f) {}

void NewConstraintWindow::render_window_content(const std::shared_ptr<ItemFocusContext> &context) {

    const auto member_names = builder_env->get_member_names();

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

                if (parent_name.has_value()) context->release_focus(parent_name.value());
                parent_name = member_names[i];
                context->focus(parent_name.value(), glm::vec3(0.7f));
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
                if (child_name.has_value()) context->release_focus(child_name.value());
                child_name = member_names[i];
                context->focus(child_name.value(), glm::vec3(0.7f));
            }

        ImGui::EndCombo();
    }

    ImGui::Spacing();
}

void NewConstraintWindow::on_close(const std::shared_ptr<ItemFocusContext> &context) {
    clear_focus(context);
}
void NewConstraintWindow::on_focus_change(
    bool new_focus, const std::shared_ptr<ItemFocusContext> &context) {
    if (new_focus) add_focus(context);
    else clear_focus(context);
}

void NewConstraintWindow::add_focus(const std::shared_ptr<ItemFocusContext> &context) {
    context->focus_black(constraint_name);

    if (parent_name.has_value()) context->focus_grey(parent_name.value());
    if (child_name.has_value()) context->focus_grey(child_name.value());
}

void NewConstraintWindow::clear_focus(const std::shared_ptr<ItemFocusContext> &context) {
    context->release_focus(constraint_name);

    if (parent_name.has_value()) context->release_focus(parent_name.value());
    if (child_name.has_value()) context->release_focus(child_name.value());
}
