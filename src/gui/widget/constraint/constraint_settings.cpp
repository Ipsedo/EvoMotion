//
// Created by samuel on 11/02/25.
//

#include <imgui.h>

#include "../window.h"

ConstraintSettingsWindow::ConstraintSettingsWindow() : ImGuiWindow("Constraint settings") {}

void ConstraintSettingsWindow::render_window_content(const std::shared_ptr<AppContext> &context) {
    if (context->is_constraint_focused()) {
        ImGui::Text("Focus on \"%s\" constraint", context->get_focused_constraint().c_str());

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        const auto constraint_type =
            context->get_builder_env()->get_constraint_type(context->get_focused_constraint());

        if (constraint_type == HINGE) {
            ImGui::Text("Hinge constraint");

            bool updated = false;

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            auto [pos, axis, limit_angle_min, limit_angle_max] =
                context->get_builder_env()->get_constraint_hinge_info(
                    context->get_focused_constraint());

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

            if (ImGui::InputFloat("axis.x", &axis.x, 0.f, 0.f, "%.8f")) updated = true;
            if (ImGui::InputFloat("axis.y", &axis.y, 0.f, 0.f, "%.8f")) updated = true;
            if (ImGui::InputFloat("axis.z", &axis.z, 0.f, 0.f, "%.8f")) updated = true;

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
                context->get_builder_env()->update_hinge_constraint(
                    context->get_focused_constraint(), pos, axis, limit_angle_min, limit_angle_max);

        } else if (constraint_type == FIXED) {
            ImGui::Text("Fixed constraint");

            bool updated = false;

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            auto [pos, rot] = context->get_builder_env()->get_constraint_fixed_info(
                context->get_focused_constraint());

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

            float rotation_angle = glm::angle(rot);
            glm::vec3 rotation_axis = glm::axis(rot);

            if (ImGui::InputFloat("axis.x", &rotation_axis.x, 0.f, 0.f, "%.8f")) {
                rotation_axis = glm::normalize(rotation_axis);
                updated = true;
            }
            if (ImGui::InputFloat("axis.y", &rotation_axis.y, 0.f, 0.f, "%.8f")) {
                rotation_axis = glm::normalize(rotation_axis);
                updated = true;
            }
            if (ImGui::InputFloat("axis.z", &rotation_axis.z, 0.f, 0.f, "%.8f")) {
                rotation_axis = glm::normalize(rotation_axis);
                updated = true;
            }
            if (ImGui::Spacing(); ImGui::InputFloat("angle", &rotation_angle, 0.f, 0.f, "%.8f"))
                updated = true;

            rot = glm::angleAxis(rotation_angle, rotation_axis);

            ImGui::EndGroup();
            ImGui::Spacing();

            if (updated)
                context->get_builder_env()->update_fixed_constraint(
                    context->get_focused_constraint(), pos, rot);
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::Button("Remove constraint")) {
            context->get_builder_env()->remove_constraint(context->get_focused_constraint());
            context->release_focus_constraint();
        }

    } else {
        ImGui::Text("No focused constraint");
    }
}
