//
// Created by samuel on 11/02/25.
//

#include "./member_construct_tools.h"

#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>

MemberConstructToolsWindow::MemberConstructToolsWindow(
    const std::string &member_name, const std::shared_ptr<RobotBuilderEnvironment> &builder_env)
    : ImGuiWindow("Construct tools of \"" + member_name + "\""), is_editing(false), changed(false),
      edit_choice(0), yaw(0.f), pitch(0.f), roll(0.f), member_name(member_name),
      builder_env(builder_env),
      construct_item(std::make_shared<NoShapeItem>(member_name + "_construct_tools", BASIS_AXIS)),
      translate_tools(std::make_unique<TranslateTools>()),
      rotate_tools(std::make_unique<RotateTools>()) {}

void MemberConstructToolsWindow::render_window_content(
    const std::shared_ptr<ItemFocusContext> &context,
    const std::shared_ptr<OpenGlWindow> &gl_window) {

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
    if (ImGui::RadioButton("Rotation", edit_choice == 1)) {
        edit_choice = 1;
        construct_item->set_drawable_kind(ROTATION_TORUS);
        changed = true;
    }
    if (ImGui::RadioButton("Scale", edit_choice == 2)) edit_choice = 2;

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Text("Axis :");
    ImGui::Spacing();

    float is_dragging_axis = false;

    ImGui::SliderFloat("yaw", &yaw, -180.f, 180.f);
    if (ImGui::IsItemActive()) is_dragging_axis = true;

    ImGui::SliderFloat("pitch", &pitch, -180.f, 180.f);
    if (ImGui::IsItemActive()) is_dragging_axis = true;

    ImGui::SliderFloat("roll", &roll, -180.f, 180.f);
    if (ImGui::IsItemActive()) is_dragging_axis = true;

    const auto [pos, rot, scale] = builder_env->get_member_transform(member_name);
    const glm::vec3 new_scale(std::max(std::max(scale.x, scale.y), scale.z));

    const auto axis_rot = glm::yawPitchRoll(
        static_cast<float>(M_PI) * yaw / 180.f, static_cast<float>(M_PI) * pitch / 180.f,
        static_cast<float>(M_PI) * roll / 180.f);

    const auto model_matrix =
        glm::translate(glm::mat4(1.f), pos) * axis_rot * glm::scale(glm::mat4(1.f), new_scale);

    construct_item->reset(model_matrix);

    switch (edit_choice) {
        case 0:
            if (const auto pos_delta_opt = translate_tools->get_pos_delta(
                    is_editing && !is_dragging_axis, {yaw, pitch, roll},
                    gl_window->get_projection_matrix(), gl_window->get_view_matrix());
                pos_delta_opt.has_value())
                builder_env->update_member(member_name, pos + pos_delta_opt.value());
            break;
        case 1:
            if (const auto rot_delta_opt = rotate_tools->get_rot_delta(
                    gl_window, is_editing && !is_dragging_axis, {yaw, pitch, roll}, pos);
                rot_delta_opt.has_value())
                builder_env->update_member(member_name, std::nullopt, rot_delta_opt.value() * rot);
            break;
        case 2: break;
    }
}

void MemberConstructToolsWindow::render_window(
    const std::shared_ptr<ItemFocusContext> &context,
    const std::shared_ptr<OpenGlWindow> &gl_window) {
    if (is_editing) {
        ImGui::SetNextWindowFocus();
        add_focus(context);
    }
    ImGuiWindow::render_window(context, gl_window);

    if (changed) {
        gl_window->remove_item(construct_item);
        if (is_editing) gl_window->add_item(construct_item);
    }
    if (is_closed()) gl_window->remove_item(construct_item);
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
bool MemberConstructToolsWindow::need_close() { return !builder_env->member_exists(member_name); }
