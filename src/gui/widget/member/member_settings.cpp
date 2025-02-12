//
// Created by samuel on 11/02/25.
//

#include "./member_settings.h"

#include <imgui.h>

#include <evo_motion_model/converter.h>

MemberSettingsWindow::MemberSettingsWindow(
    const std::string &member_name, const std::shared_ptr<RobotBuilderEnvironment> &builder_env)
    : ImGuiWindow("Member settings \"" + member_name + "\""), member_name(member_name),
      builder_env(builder_env) {}

void MemberSettingsWindow::on_close(const std::shared_ptr<ItemFocusContext> &context) {
    clear_focus(context);
}

void MemberSettingsWindow::render_window_content(const std::shared_ptr<ItemFocusContext> &context) {

    auto [member_pos, member_rot, member_scale] = builder_env->get_member_transform(member_name);

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
    std::string new_member_name = member_name;
    new_member_name.resize(128);
    if (ImGui::InputText("Member name", &new_member_name[0], new_member_name.size()))
        if (builder_env->rename_member(member_name, new_member_name.c_str())) {
            clear_focus(context);
            member_name = new_member_name.c_str();
            add_focus(context);
        }

    ImGui::Spacing();

    // mass
    float mass = builder_env->get_member_mass(member_name);
    if (ImGui::InputFloat("mass (kg)", &mass, 0.f, 0.f, "%.8f")) updated = true;

    ImGui::Spacing();

    // friction
    float friction = builder_env->get_member_friction(member_name);
    if (ImGui::DragFloat("friction", &friction, 0.01f, 0.f, 1.f)) updated = true;

    ImGui::Columns(1);

    if (updated)
        builder_env->update_member(
            member_name, member_pos, member_rot, member_scale, friction, mass);

    // remove
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::Button("Remove member")) {
        builder_env->remove_member(member_name);
        clear_focus(context);
        close();
    }
}

void MemberSettingsWindow::on_focus_change(
    bool new_focus, const std::shared_ptr<ItemFocusContext> &context) {
    if (new_focus) add_focus(context);
    else clear_focus(context);
}

void MemberSettingsWindow::add_focus(const std::shared_ptr<ItemFocusContext> &context) {
    context->focus(member_name, glm::vec3(0.f));
}
void MemberSettingsWindow::clear_focus(const std::shared_ptr<ItemFocusContext> &context) {
    context->release_focus(member_name);
}
