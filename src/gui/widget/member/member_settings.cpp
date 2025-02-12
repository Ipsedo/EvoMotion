//
// Created by samuel on 11/02/25.
//

#include <imgui.h>

#include <evo_motion_model/converter.h>

#include "../window.h"

MemberSettingsWindow::MemberSettingsWindow() : ImGuiWindow("Member settings") {}

void MemberSettingsWindow::render_window_content(const std::shared_ptr<AppContext> &context) {
    if (context->focused_member.is_set()) {

        auto focused_member_name = context->focused_member.get();
        const auto builder_env = context->builder_env.get();

        ImGui::Text("Focus on \"%s\" member", focused_member_name.c_str());

        auto [member_pos, member_rot, member_scale] =
            builder_env->get_member_transform(focused_member_name);

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::Columns(2, nullptr, false);

        // Position
        ImGui::Spacing();

        ImGui::BeginGroup();
        ImGui::Text("Position");
        ImGui::Spacing();

        bool updated = false;

        if (ImGui::InputFloat("pos.x", &member_pos.x, 0.f, 0.f, "%.8f")) updated = true;
        if (ImGui::InputFloat("pos.y", &member_pos.y, 0.f, 0.f, "%.8f")) updated = true;
        if (ImGui::InputFloat("pos.z", &member_pos.z, 0.f, 0.f, "%.8f")) updated = true;

        ImGui::EndGroup();

        // Rotation
        ImGui::Spacing();

        ImGui::BeginGroup();

        ImGui::Text("Rotation");
        ImGui::Spacing();

        auto [rotation_axis, rotation_angle] = quat_to_axis_angle(member_rot);

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

        member_rot = axis_angle_to_quat(rotation_axis, rotation_angle);

        ImGui::EndGroup();

        // Scaling
        ImGui::Spacing();

        ImGui::BeginGroup();

        ImGui::Text("Scale");
        ImGui::Spacing();

        float min_scale = 1e-4f;

        if (ImGui::InputFloat("scale.x", &member_scale.x, 0.f, 0.f, "%.4f")) {
            member_scale.x = std::max(member_scale.x, min_scale);
            updated = true;
        }
        if (ImGui::InputFloat("scale.y", &member_scale.y, 0.f, 0.f, "%.4f")) {
            member_scale.y = std::max(member_scale.y, min_scale);
            updated = true;
        }
        if (ImGui::InputFloat("scale.z", &member_scale.z, 0.f, 0.f, "%.4f")) {
            member_scale.z = std::max(member_scale.z, min_scale);
            updated = true;
        }

        ImGui::EndGroup();

        ImGui::NextColumn();

        // Member name
        ImGui::Spacing();
        ImGui::Text("Member parameters");
        ImGui::Spacing();

        // Member name
        std::string member_name = focused_member_name;
        member_name.resize(128);
        if (ImGui::InputText("Member name", &member_name[0], member_name.size()))
            if (context->builder_env.get()->rename_member(
                    focused_member_name, member_name.c_str())) {
                context->focused_member.set(member_name.c_str());
                focused_member_name = context->focused_member.get();
            }

        ImGui::Spacing();

        // mass
        float mass = builder_env->get_member_mass(focused_member_name);
        if (ImGui::InputFloat("mass (kg)", &mass, 0.f, 0.f, "%.8f")) updated = true;

        ImGui::Spacing();

        // friction
        float friction = builder_env->get_member_friction(focused_member_name);
        if (ImGui::DragFloat("friction", &friction, 0.01f, 0.f, 1.f)) updated = true;

        ImGui::Columns(1);

        if (updated)
            builder_env->update_member(
                focused_member_name, member_pos, member_rot, member_scale, friction, mass);

        // remove
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::Button("Remove member")) {
            builder_env->remove_member(focused_member_name);
            context->focused_member.release();
        }

    } else {
        ImGui::Text("No focus member");
    }
}
