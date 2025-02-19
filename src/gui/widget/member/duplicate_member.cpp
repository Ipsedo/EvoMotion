//
// Created by samuel on 19/02/25.
//

#include "./duplicate_member.h"

#include "../utils.h"

DuplicateGroupWindow::DuplicateGroupWindow(
    const std::string &member_name, const std::shared_ptr<RobotBuilderEnvironment> &builder_env)
    : ImGuiWindow("Duplicate \"" + member_name + "\""), member_name(member_name),
      builder_env(builder_env), pos(0), rotation_axis(0, 1, 0), rotation_angle(0.f),
      prefix_name("clone_") {}

void DuplicateGroupWindow::add_focus(const std::shared_ptr<ItemFocusContext> &context) const {
    context->focus_black(member_name);
}

void DuplicateGroupWindow::clear_focus(const std::shared_ptr<ItemFocusContext> &context) const {
    context->release_focus(member_name);
}

void DuplicateGroupWindow::render_window_content(
    const std::shared_ptr<ItemFocusContext> &context,
    const std::shared_ptr<OpenGlWindow> &gl_window) {
    prefix_name.resize(128);
    ImGui::InputText("Prefix", &prefix_name[0], prefix_name.size());
    prefix_name = prefix_name.c_str();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // position
    ImGui::BeginGroup();
    ImGui::Spacing();

    ImGui::Text("Position");
    ImGui::Spacing();

    input_float("pos.x", &pos.x, 4);
    input_float("pos.y", &pos.y, 4);
    input_float("pos.z", &pos.z, 4);

    ImGui::EndGroup();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // rotation
    ImGui::BeginGroup();
    ImGui::Spacing();

    ImGui::Text("Rotation");
    ImGui::Spacing();

    if (input_float("axis.x", &rotation_axis.x, 4))
        rotation_axis = glm::normalize(rotation_axis + 1e-5f);
    if (input_float("axis.y", &rotation_axis.y, 4))
        rotation_axis = glm::normalize(rotation_axis + 1e-5f);
    if (input_float("axis.z", &rotation_axis.z, 4))
        rotation_axis = glm::normalize(rotation_axis + 1e-5f);

    rotation_angle = glm::degrees(rotation_angle);
    input_float("angle", &rotation_angle, 4);
    rotation_angle = glm::radians(rotation_angle);

    ImGui::EndGroup();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::Button("Duplicate")
        && builder_env->clone_body_part(
            member_name, prefix_name, pos, glm::angleAxis(rotation_angle, rotation_axis)))
        close();
}

void DuplicateGroupWindow::on_close(const std::shared_ptr<ItemFocusContext> &context) {
    clear_focus(context);
}

void DuplicateGroupWindow::on_focus_change(
    bool new_focus, const std::shared_ptr<ItemFocusContext> &context) {
    if (new_focus) add_focus(context);
    else clear_focus(context);
}
