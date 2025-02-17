//
// Created by samuel on 11/02/25.
//

#include "./member_construct_tools.h"

#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>

#include "../construct/rotate.h"

MemberConstructToolsWindow::MemberConstructToolsWindow(
    const std::string &member_name, const std::shared_ptr<RobotBuilderEnvironment> &builder_env)
    : ImGuiWindow("Construct tools of \"" + member_name + "\""), is_editing(false), changed(false),
      edit_choice(0), yaw(0.f), pitch(0.f), roll(0.f), member_name(member_name),
      builder_env(builder_env),
      construct_item(std::make_shared<NoShapeItem>(member_name + "_construct_tools", BASIS_AXIS)) {}

void MemberConstructToolsWindow::render_window_content(
    const std::shared_ptr<ItemFocusContext> &context) {

    ImGui::Spacing();

    changed = false;

    if (ImGui::Button(is_editing ? "Edit : ON" : "Edit : OFF")) {
        is_editing = !is_editing;
        changed = true;
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Text("Edit mode :");
    ImGui::Spacing();

    if (ImGui::RadioButton("Translation", edit_choice == 0)) {
        edit_choice = 0;
        construct_item->set_drawable_kind(BASIS_AXIS);
        changed = true;
    }
    if (ImGui::RadioButton("Rotation", edit_choice == 1)) edit_choice = 1;
    if (ImGui::RadioButton("Scale", edit_choice == 2)) edit_choice = 2;

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Text("Axis :");
    ImGui::Spacing();

    ImGui::SliderFloat("yaw", &yaw, -180.f, 180.f);
    ImGui::SliderFloat("pitch", &pitch, -180.f, 180.f);
    ImGui::SliderFloat("roll", &roll, -180.f, 180.f);

    const auto [pos, rot, scale] = builder_env->get_member_transform(member_name);
    const glm::vec3 new_scale(std::max(std::max(scale.x, scale.y), scale.z));
    const auto model_matrix = glm::translate(glm::mat4(1.f), pos) * glm::toMat4(rot)
                              * glm::scale(glm::mat4(1.f), new_scale);

    construct_item->reset(model_matrix);
}

void MemberConstructToolsWindow::render_window(const std::shared_ptr<ItemFocusContext> &context) {
    if (is_editing) {
        ImGui::SetNextWindowFocus();
        add_focus(context);
    }
    ImGuiWindow::render_window(context);
}

void MemberConstructToolsWindow::on_render(const std::shared_ptr<OpenGlWindow> &gl_window) {
    if (changed) {
        gl_window->remove_item(construct_item);
        if (is_editing) gl_window->add_item(construct_item);
    }
    ImGuiWindow::on_render(gl_window);
}

void MemberConstructToolsWindow::on_close(const std::shared_ptr<ItemFocusContext> &context) {
    clear_focus(context);
}

void MemberConstructToolsWindow::on_focus_change(
    bool new_focus, const std::shared_ptr<ItemFocusContext> &context) {
    if (new_focus) add_focus(context);
    else clear_focus(context);
}

void MemberConstructToolsWindow::add_focus(const std::shared_ptr<ItemFocusContext> &context) {
    context->focus_black(member_name);
}

void MemberConstructToolsWindow::clear_focus(const std::shared_ptr<ItemFocusContext> &context) {
    context->release_focus(member_name);
}
