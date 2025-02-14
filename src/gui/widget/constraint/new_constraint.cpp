//
// Created by samuel on 11/02/25.
//

#include "./new_constraint.h"

#include <imgui.h>

#include "../utils.h"

NewConstraintWindow::NewConstraintWindow(
    const ConstraintType &constraint_type,
    const std::shared_ptr<RobotBuilderEnvironment> &builder_env)
    : ImGuiWindow(NewConstraintWindow::get_window_name(constraint_type)), builder_env(builder_env),
      constraint_name("no_name_constraint"), parent_name(std::nullopt), child_name(std::nullopt),
      absolute_position(0.f) {}

void NewConstraintWindow::render_window_content(const std::shared_ptr<ItemFocusContext> &context) {

    const auto member_names = builder_env->get_member_names();

    // constraint name
    constraint_name.resize(128);
    input_text("Name", &constraint_name[0], constraint_name.size(), 16);
    constraint_name = constraint_name.c_str();

    ImGui::Spacing();

    // combo size
    const ImVec2 parent_title_size = ImGui::CalcTextSize("Parent");
    const ImVec2 default_parent_size = ImGui::CalcTextSize("No parent");

    const ImVec2 child_title_size = ImGui::CalcTextSize("Child");
    const ImVec2 default_child_size = ImGui::CalcTextSize("No child");

    const float combo_width = std::max(
        parent_title_size.x + default_parent_size.x + ImGui::GetStyle().FramePadding.x * 4,
        child_title_size.x + default_child_size.x + ImGui::GetStyle().FramePadding.x * 4);

    // parent
    ImGui::SetNextItemWidth(combo_width);
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
    ImGui::SetNextItemWidth(combo_width);
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
    ImGui::Separator();
    ImGui::Spacing();

    render_constraint_specific_settings(
        context, builder_env, constraint_name, parent_name, child_name, absolute_position);
}

void NewConstraintWindow::on_close(const std::shared_ptr<ItemFocusContext> &context) {
    clear_focus(context);
}
void NewConstraintWindow::on_focus_change(
    const bool new_focus, const std::shared_ptr<ItemFocusContext> &context) {
    if (new_focus) add_focus(context);
    else clear_focus(context);
}

void NewConstraintWindow::add_focus(const std::shared_ptr<ItemFocusContext> &context) const {
    context->focus_black(constraint_name);

    if (parent_name.has_value()) context->focus_grey(parent_name.value());
    if (child_name.has_value()) context->focus_grey(child_name.value());
}

void NewConstraintWindow::clear_focus(const std::shared_ptr<ItemFocusContext> &context) const {
    context->release_focus(constraint_name);

    if (parent_name.has_value()) context->release_focus(parent_name.value());
    if (child_name.has_value()) context->release_focus(child_name.value());
}

std::string NewConstraintWindow::get_window_name(const ConstraintType &constraint_type) {
    std::string window_name = "New ";

    switch (constraint_type) {
        case HINGE: window_name += "hinge"; break;
        case FIXED: window_name += "fixed";
    }

    return window_name + " constraint";
}

/*
 * Fixed
 */

NewFixedConstraintWindow::NewFixedConstraintWindow(
    const std::shared_ptr<RobotBuilderEnvironment> &builder_env)
    : NewConstraintWindow(FIXED, builder_env), rotation_axis(1, 0, 0), angle(0.f) {}

void NewFixedConstraintWindow::render_constraint_specific_settings(
    const std::shared_ptr<ItemFocusContext> &context,
    const std::shared_ptr<RobotBuilderEnvironment> &builder_env, const std::string &constraint_name,
    const std::optional<std::string> &parent_name, const std::optional<std::string> &child_name,
    const glm::vec3 &absolute_position) {}

/*
 * Hinge
 */

NewHingeConstraintWindow::NewHingeConstraintWindow(
    const std::shared_ptr<RobotBuilderEnvironment> &builder_env)
    : NewConstraintWindow(HINGE, builder_env), hinge_axis(1, 0, 0) {}

void NewHingeConstraintWindow::render_constraint_specific_settings(
    const std::shared_ptr<ItemFocusContext> &context,
    const std::shared_ptr<RobotBuilderEnvironment> &builder_env, const std::string &constraint_name,
    const std::optional<std::string> &parent_name, const std::optional<std::string> &child_name,
    const glm::vec3 &absolute_position) {}
