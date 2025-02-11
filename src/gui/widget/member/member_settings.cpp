//
// Created by samuel on 11/02/25.
//

#include <imgui.h>

#include "../window.h"

MemberSettingsWindow::MemberSettingsWindow() : ImGuiWindow("Member settings") {}

void MemberSettingsWindow::render_window_content(const std::shared_ptr<AppContext> &context) {
    if (context->is_member_focused()) {
        ImGui::Text("Focus on \"%s\" member", context->get_focused_member().c_str());

        auto [member_pos, member_rot, member_scale] =
            context->get_builder_env()->get_member_transform(context->get_focused_member());

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

        float rotation_angle = glm::angle(member_rot);
        glm::vec3 rotation_axis = glm::axis(member_rot);

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

        member_rot = glm::angleAxis(rotation_angle, rotation_axis);

        ImGui::EndGroup();

        // Scaling
        ImGui::Spacing();

        ImGui::BeginGroup();

        ImGui::Text("Scale");
        ImGui::Spacing();

        if (ImGui::InputFloat("scale.x", &member_scale.x, 0.f, 0.f, "%.8f")) {
            member_scale.x = std::max(member_scale.x, 1e-8f);
            updated = true;
        }
        if (ImGui::InputFloat("scale.y", &member_scale.y, 0.f, 0.f, "%.8f")) {
            member_scale.y = std::max(member_scale.y, 1e-8f);
            updated = true;
        }
        if (ImGui::InputFloat("scale.z", &member_scale.z, 0.f, 0.f, "%.8f")) {
            member_scale.z = std::max(member_scale.z, 1e-8f);
            updated = true;
        }

        ImGui::EndGroup();

        ImGui::NextColumn();

        // Member name
        ImGui::Spacing();
        ImGui::Text("Member parameters");
        ImGui::Spacing();

        // Member name
        std::string member_name = context->get_focused_member();
        member_name.resize(128);
        if (ImGui::InputText("Member name", &member_name[0], member_name.size()))
            if (context->get_builder_env()->rename_member(
                    context->get_focused_member(), member_name.c_str()))
                context->set_focus_member(member_name.c_str());

        ImGui::Spacing();

        // mass
        float mass = context->get_builder_env()->get_member_mass(context->get_focused_member());
        if (ImGui::InputFloat("mass (kg)", &mass, 0.f, 0.f, "%.8f")) updated = true;

        ImGui::Spacing();

        // friction
        float friction =
            context->get_builder_env()->get_member_friction(context->get_focused_member());
        if (ImGui::DragFloat("friction", &friction, 0.01f, 0.f, 1.f)) updated = true;

        ImGui::Columns(1);

        if (updated)
            context->get_builder_env()->update_member(
                context->get_focused_member(), member_pos, member_rot, member_scale, friction,
                mass);

        // remove
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::Button("Remove member")) {
            context->get_builder_env()->remove_member(context->get_focused_member());
            context->release_focus_member();
        }

    } else {
        ImGui::Text("No focus member");
    }
}
