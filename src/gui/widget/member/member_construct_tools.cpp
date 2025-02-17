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
      edit_choice(0), is_dragging(false), prev_x_mouse(0.f), prev_y_mouse(0.f), yaw(0.f),
      pitch(0.f), roll(0.f), member_name(member_name), builder_env(builder_env),
      construct_item(std::make_shared<NoShapeItem>(member_name + "_construct_tools", BASIS_AXIS)) {}

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
    if (ImGui::RadioButton("Rotation", edit_choice == 1)) edit_choice = 1;
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

    if (is_editing && ImGui::IsMouseDragging(ImGuiMouseButton_Left) && !ImGui::IsWindowHovered()
        && !is_dragging_axis) {

        if (!is_dragging) {
            is_dragging = true;
            const auto mouse_pos = ImGui::GetMousePos();
            prev_x_mouse = mouse_pos.x;
            prev_y_mouse = mouse_pos.y;
        }

        glm::mat4 mvp_matrix =
            gl_window->get_projection_matrix() * gl_window->get_view_matrix() * axis_rot;

        glm::vec3 proj_x = mvp_matrix * glm::vec4(1, 0, 0, 0);
        glm::vec3 proj_y = mvp_matrix * glm::vec4(0, 1, 0, 0);
        glm::vec3 proj_z = mvp_matrix * glm::vec4(0, 0, 1, 0);

        const auto new_mouse_pos = ImGui::GetMousePos();
        float screen_move_x = (new_mouse_pos.x - prev_x_mouse) / 100.f,
              screen_move_y = (new_mouse_pos.y - prev_y_mouse) / 100.f;

        float mouse_threshold = 1e-2f;

        if (std::abs(screen_move_x) < mouse_threshold) screen_move_x = 0.f;
        else screen_move_x += screen_move_x < 0 ? mouse_threshold : -mouse_threshold;
        if (std::abs(screen_move_y) < mouse_threshold) screen_move_y = 0.f;
        else screen_move_y += screen_move_y < 0 ? mouse_threshold : -mouse_threshold;

        float x_x = std::abs(proj_x.x), x_y = std::abs(proj_x.y);
        float y_x = std::abs(proj_y.x), y_y = std::abs(proj_y.y);
        float z_x = std::abs(proj_z.x), z_y = std::abs(proj_z.y);

        glm::vec3 new_move(0.f);

        std::string horizontal_axis;
        std::string vertical_axis;

        // horizontal
        if (x_x >= y_x && x_x >= z_x) {
            horizontal_axis = "x";
            new_move.x = proj_x.x > 0 ? screen_move_x : -screen_move_x;
        } else if (y_x >= x_x && y_x >= z_x) {
            horizontal_axis = "y";
            new_move.y = proj_y.x > 0 ? screen_move_x : -screen_move_x;
        } else {
            horizontal_axis = "z";
            new_move.z = proj_z.x > 0 ? screen_move_x : -screen_move_x;
        }

        // vertical
        if (x_y >= y_y && x_y >= z_y) vertical_axis = "x";
        else if (y_y >= x_y && y_y >= z_y) vertical_axis = "y";
        else vertical_axis = "z";

        if (!ImGui::IsKeyDown(ImGuiKey_LeftShift)) {
            if (vertical_axis == "x") new_move.x = proj_x.y > 0 ? -screen_move_y : screen_move_y;
            else if (vertical_axis == "y")
                new_move.y = proj_y.y > 0 ? -screen_move_y : screen_move_y;
            else if (vertical_axis == "z")
                new_move.z = proj_z.y > 0 ? -screen_move_y : screen_move_y;
        } else {
            if (vertical_axis == "x" && horizontal_axis == "y")
                new_move.z = proj_x.y > 0 ? screen_move_y : -screen_move_y;
            else if (vertical_axis == "x" && horizontal_axis == "z")
                new_move.y = proj_x.y > 0 ? screen_move_y : -screen_move_y;

            else if (vertical_axis == "y" && horizontal_axis == "x")
                new_move.z = proj_y.y > 0 ? screen_move_y : -screen_move_y;
            else if (vertical_axis == "y" && horizontal_axis == "z")
                new_move.x = proj_y.y > 0 ? screen_move_y : -screen_move_y;

            else if (vertical_axis == "z" && horizontal_axis == "x")
                new_move.y = proj_z.y > 0 ? screen_move_y : -screen_move_y;
            else if (vertical_axis == "z" && horizontal_axis == "x")
                new_move.y = proj_z.y > 0 ? screen_move_y : -screen_move_y;
        }

        glm::vec3 new_pos = pos + glm::vec3(axis_rot * glm::vec4(new_move, 0.f));

        prev_x_mouse = new_mouse_pos.x;
        prev_y_mouse = new_mouse_pos.y;

        builder_env->update_member(member_name, new_pos);
    } else {
        is_dragging = false;
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
