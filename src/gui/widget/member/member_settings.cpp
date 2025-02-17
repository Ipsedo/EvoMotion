//
// Created by samuel on 11/02/25.
//

#include "./member_settings.h"

#include <imgui.h>

#include <evo_motion_model/converter.h>

#include "../utils.h"

MemberSettingsWindow::MemberSettingsWindow(
    const std::string &member_name, const std::shared_ptr<RobotBuilderEnvironment> &builder_env)
    : ImGuiWindow("Member settings of \"" + member_name + "\""), member_name(member_name),
      builder_env(builder_env) {}

void MemberSettingsWindow::on_close(const std::shared_ptr<ItemFocusContext> &context) {
    clear_focus(context);
}

void MemberSettingsWindow::render_window_content(
    const std::shared_ptr<ItemFocusContext> &context,
    const std::shared_ptr<OpenGlWindow> &gl_window) {
    auto [member_pos, member_rot, member_scale] = builder_env->get_member_transform(member_name);

    // Position
    ImGui::Spacing();
    ImGui::Text("Position");
    ImGui::Spacing();

    bool updated = false;

    if (input_float("pos.x", &member_pos.x, 4)) updated = true;
    if (input_float("pos.y", &member_pos.y, 4)) updated = true;
    if (input_float("pos.z", &member_pos.z, 4)) updated = true;

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Rotation
    ImGui::Spacing();
    ImGui::Text("Rotation");
    ImGui::Spacing();

    auto [rotation_axis, rotation_angle] = quat_to_axis_angle(member_rot);

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

    member_rot = axis_angle_to_quat(rotation_axis, rotation_angle);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Scaling
    ImGui::Spacing();
    ImGui::Text("Scale");
    ImGui::Spacing();

    constexpr float min_scale = 1e-4f;

    if (input_float("scale.x", &member_scale.x, 4)) {
        member_scale.x = std::max(member_scale.x, min_scale);
        updated = true;
    }
    if (input_float("scale.y", &member_scale.y, 4)) {
        member_scale.y = std::max(member_scale.y, min_scale);
        updated = true;
    }
    if (input_float("scale.z", &member_scale.z, 4)) {
        member_scale.z = std::max(member_scale.z, min_scale);
        updated = true;
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

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
    if (input_float("mass (kg)", &mass, 4)) updated = true;

    ImGui::Spacing();

    // friction
    float friction = builder_env->get_member_friction(member_name);
    if (ImGui::SliderFloat("friction", &friction, 0.f, 1.f)) updated = true;

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
    const bool new_focus, const std::shared_ptr<ItemFocusContext> &context) {
    if (new_focus) add_focus(context);
    else clear_focus(context);
}

void MemberSettingsWindow::add_focus(const std::shared_ptr<ItemFocusContext> &context) const {
    context->focus_black(member_name);
}

void MemberSettingsWindow::clear_focus(const std::shared_ptr<ItemFocusContext> &context) const {
    context->release_focus(member_name);
}
