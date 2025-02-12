//
// Created by samuel on 11/02/25.
//

#include "./constraint_settings.h"

#include <imgui.h>

#include <evo_motion_model/converter.h>

ConstraintSettingsWindow::ConstraintSettingsWindow(
    const std::string &constraint_name, const std::shared_ptr<RobotBuilderEnvironment> &builder_env)
    : ImGuiWindow("Constraint settings"), constraint_name(constraint_name),
      builder_env(builder_env), parent_name(), child_name() {
    auto [p_n, c_n] = builder_env->get_constraint_members(constraint_name);
    parent_name = p_n;
    child_name = c_n;
}

void ConstraintSettingsWindow::render_window_content(
    const std::shared_ptr<ItemFocusContext> &context) {

    ImGui::Text("Focus on \"%s\" constraint", constraint_name.c_str());

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    const auto constraint_type = builder_env->get_constraint_type(constraint_name);

    if (constraint_type == HINGE) {
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

        if (ImGui::InputFloat("pos.x", &pos.x, 0.f, 0.f, "%.8f")) updated = true;
        if (ImGui::InputFloat("pos.y", &pos.y, 0.f, 0.f, "%.8f")) updated = true;
        if (ImGui::InputFloat("pos.z", &pos.z, 0.f, 0.f, "%.8f")) updated = true;

        ImGui::EndGroup();
        ImGui::Spacing();

        // axis
        ImGui::BeginGroup();
        ImGui::Text("Axis");
        ImGui::Spacing();

        if (ImGui::InputFloat("axis.x", &axis.x, 0.f, 0.f, "%.8f")) {
            axis = glm::normalize(axis);
            updated = true;
        }
        if (ImGui::InputFloat("axis.y", &axis.y, 0.f, 0.f, "%.8f")) {
            axis = glm::normalize(axis);
            updated = true;
        }
        if (ImGui::InputFloat("axis.z", &axis.z, 0.f, 0.f, "%.8f")) {
            axis = glm::normalize(axis);
            updated = true;
        }

        ImGui::EndGroup();
        ImGui::Spacing();

        // Limit
        ImGui::BeginGroup();
        ImGui::Text("Angular limits");
        ImGui::Spacing();

        if (ImGui::InputFloat("min", &limit_angle_min, 0.f, 0.f, "%.8f")) updated = true;
        if (ImGui::InputFloat("max", &limit_angle_max, 0.f, 0.f, "%.8f")) updated = true;

        ImGui::EndGroup();

        // final
        if (updated)
            builder_env->update_hinge_constraint(
                constraint_name, pos, axis, limit_angle_min, limit_angle_max);

    } else if (constraint_type == FIXED) {
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

        if (ImGui::InputFloat("pos.x", &pos.x, 0.f, 0.f, "%.8f")) updated = true;
        if (ImGui::InputFloat("pos.y", &pos.y, 0.f, 0.f, "%.8f")) updated = true;
        if (ImGui::InputFloat("pos.z", &pos.z, 0.f, 0.f, "%.8f")) updated = true;

        ImGui::EndGroup();
        ImGui::Spacing();

        // axis
        ImGui::BeginGroup();
        ImGui::Text("Rotation");
        ImGui::Spacing();

        auto [rotation_axis, rotation_angle] = quat_to_axis_angle(rot);

        if (ImGui::InputFloat("axis.x", &rotation_axis.x, 0.f, 0.f, "%.8f")) {
            rotation_axis = glm::normalize(rotation_axis + 1e-9f);
            updated = true;
        }
        if (ImGui::InputFloat("axis.y", &rotation_axis.y, 0.f, 0.f, "%.8f")) {
            rotation_axis = glm::normalize(rotation_axis + 1e-9f);
            updated = true;
        }
        if (ImGui::InputFloat("axis.z", &rotation_axis.z, 0.f, 0.f, "%.8f")) {
            rotation_axis = glm::normalize(rotation_axis + 1e-9f);
            updated = true;
        }
        if (ImGui::InputFloat("angle", &rotation_angle, 0.f, 0.f, "%.8f")) updated = true;

        rot = axis_angle_to_quat(rotation_axis, rotation_angle);

        ImGui::EndGroup();
        ImGui::Spacing();

        if (updated) builder_env->update_fixed_constraint(constraint_name, pos, rot);
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::Button("Remove constraint")) {
        builder_env->remove_constraint(constraint_name);
        clear_focus(context);
        close();
    }
}

void ConstraintSettingsWindow::add_focus(const std::shared_ptr<ItemFocusContext> &context) {
    context->focus(constraint_name, glm::vec3(0.f));

    context->focus(parent_name, glm::vec3(0.7f));
    context->focus(child_name, glm::vec3(0.7f));
}

void ConstraintSettingsWindow::clear_focus(const std::shared_ptr<ItemFocusContext> &context) {
    context->release_focus(constraint_name);
    context->release_focus(parent_name);
    context->release_focus(child_name);
}

void ConstraintSettingsWindow::on_close(const std::shared_ptr<ItemFocusContext> &context) {
    clear_focus(context);
}

void ConstraintSettingsWindow::on_focus_change(
    bool new_focus, const std::shared_ptr<ItemFocusContext> &context) {
    if (new_focus) add_focus(context);
    else clear_focus(context);
}
