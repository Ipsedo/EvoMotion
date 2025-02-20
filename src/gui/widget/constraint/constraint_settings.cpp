//
// Created by samuel on 11/02/25.
//

#include "./constraint_settings.h"

#include <imgui.h>

#include <evo_motion_model/converter.h>

#include "../utils.h"

ConstraintSettingsWindow::ConstraintSettingsWindow(
    const std::string &constraint_name, const std::shared_ptr<RobotBuilderEnvironment> &builder_env)
    : ImGuiWindow("Constraint settings of \"" + constraint_name + "\""),
      constraint_name(constraint_name), parent_name(), child_name(), builder_env(builder_env) {
    auto [p_n, c_n] = builder_env->get_constraint_members(constraint_name);
    parent_name = p_n;
    child_name = c_n;
}

void ConstraintSettingsWindow::render_window_content(
    const std::shared_ptr<ItemFocusContext> &context,
    const std::shared_ptr<OpenGlWindow> &gl_window) {

    render_constraint_specific_window(constraint_name, builder_env, context);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::Button("Remove constraint")) {
        builder_env->remove_constraint(constraint_name);
        clear_focus(context);
        close();
    }
}

void ConstraintSettingsWindow::add_focus(const std::shared_ptr<ItemFocusContext> &context) const {
    context->focus_black(constraint_name);

    context->focus_grey(parent_name);
    context->focus_grey(child_name);
}

void ConstraintSettingsWindow::clear_focus(const std::shared_ptr<ItemFocusContext> &context) const {
    context->release_focus(constraint_name);
    context->release_focus(parent_name);
    context->release_focus(child_name);
}

void ConstraintSettingsWindow::on_close(const std::shared_ptr<ItemFocusContext> &context) {
    clear_focus(context);
}

void ConstraintSettingsWindow::on_focus_change(
    const bool new_focus, const std::shared_ptr<ItemFocusContext> &context) {
    if (new_focus) add_focus(context);
    else clear_focus(context);
}

bool ConstraintSettingsWindow::need_close() {
    return !builder_env->constraint_exists(constraint_name);
}

/*
 * Hinge
 */

HingeConstraintSettingsWindow::HingeConstraintSettingsWindow(
    const std::string &constraint_name, const std::shared_ptr<RobotBuilderEnvironment> &builder_env)
    : ConstraintSettingsWindow(constraint_name, builder_env) {}

void HingeConstraintSettingsWindow::render_constraint_specific_window(
    const std::string &constraint_name, const std::shared_ptr<RobotBuilderEnvironment> &builder_env,
    const std::shared_ptr<ItemFocusContext> &context) {
    ImGui::Text("Hinge constraint");

    bool updated = false;

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    auto [pos, axis, limit_angle_min, limit_angle_max] =
        builder_env->get_constraint_hinge_info(constraint_name);

    // position
    ImGui::BeginGroup();
    ImGui::Text("Position");
    ImGui::Spacing();

    if (input_float("pos.x", &pos.x, 4)) updated = true;
    if (input_float("pos.y", &pos.y, 4)) updated = true;
    if (input_float("pos.z", &pos.z, 4)) updated = true;

    ImGui::EndGroup();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // axis
    ImGui::BeginGroup();
    ImGui::Text("Axis");
    ImGui::Spacing();

    if (input_float("axis.x", &axis.x, 4)) {
        axis = glm::normalize(axis);
        updated = true;
    }
    if (input_float("axis.y", &axis.y, 4)) {
        axis = glm::normalize(axis);
        updated = true;
    }
    if (input_float("axis.z", &axis.z, 4)) {
        axis = glm::normalize(axis);
        updated = true;
    }

    ImGui::EndGroup();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Limit
    ImGui::BeginGroup();
    ImGui::Text("Angular limits");
    ImGui::Spacing();

    limit_angle_min = glm::degrees(limit_angle_min);
    limit_angle_max = glm::degrees(limit_angle_max);
    if (input_float("min", &limit_angle_min, 4)) updated = true;
    if (input_float("max", &limit_angle_max, 4)) updated = true;
    limit_angle_min = glm::radians(limit_angle_min);
    limit_angle_max = glm::radians(limit_angle_max);

    ImGui::EndGroup();

    // final
    if (updated)
        builder_env->update_hinge_constraint(
            constraint_name, pos, axis, limit_angle_min, limit_angle_max);
}

/*
 * Fixed
 */

FixedConstraintSettingsWindow::FixedConstraintSettingsWindow(
    const std::string &constraint_name, const std::shared_ptr<RobotBuilderEnvironment> &builder_env)
    : ConstraintSettingsWindow(constraint_name, builder_env) {}

void FixedConstraintSettingsWindow::render_constraint_specific_window(
    const std::string &constraint_name, const std::shared_ptr<RobotBuilderEnvironment> &builder_env,
    const std::shared_ptr<ItemFocusContext> &context) {
    ImGui::Text("Fixed constraint");

    bool updated = false;

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    auto [pos, rot] = builder_env->get_constraint_fixed_info(constraint_name);

    // position
    ImGui::BeginGroup();
    ImGui::Text("Position");
    ImGui::Spacing();

    if (input_float("pos.x", &pos.x, 4)) updated = true;
    if (input_float("pos.y", &pos.y, 4)) updated = true;
    if (input_float("pos.z", &pos.z, 4)) updated = true;

    ImGui::EndGroup();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // axis
    ImGui::BeginGroup();
    ImGui::Text("Rotation");
    ImGui::Spacing();

    auto [rotation_axis, rotation_angle] = quat_to_axis_angle(rot);

    if (input_float("axis.x", &rotation_axis.x, 4)) {
        rotation_axis = glm::normalize(rotation_axis + 1e-5f);
        updated = true;
    }
    if (input_float("axis.y", &rotation_axis.y, 4)) {
        rotation_axis = glm::normalize(rotation_axis + 1e-5f);
        updated = true;
    }
    if (input_float("axis.z", &rotation_axis.z, 4)) {
        rotation_axis = glm::normalize(rotation_axis + 1e-5f);
        updated = true;
    }
    if (input_float("angle", &rotation_angle, 4)) updated = true;

    rot = axis_angle_to_quat(rotation_axis, rotation_angle);

    ImGui::EndGroup();
    ImGui::Spacing();

    if (updated) builder_env->update_fixed_constraint(constraint_name, pos, rot);
}
