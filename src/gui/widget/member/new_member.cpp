//
// Created by samuel on 11/02/25.
//

#include "./new_member.h"

#include <evo_motion_model/converter.h>

#include "../utils.h"

NewMemberWindow::NewMemberWindow(const std::shared_ptr<RobotBuilderEnvironment> &builder_env)
    : ImGuiWindow("New member"), builder_env(builder_env), member_name("no_name_member"), pos(0),
      rotation_axis(0, 1, 0), rotation_angle(0.f), scale(1.f), mass(1.f), friction(0.5f) {}

void NewMemberWindow::render_window_content(const std::shared_ptr<ItemFocusContext> &context) {
    member_name.resize(128);
    input_text("Name", &member_name[0], member_name.size(), 16);
    member_name = member_name.c_str();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // position
    ImGui::BeginGroup();
    ImGui::Spacing();

    ImGui::Text("Position");
    ImGui::Spacing();

    input_float("pos.x", &pos.x, 8);
    input_float("pos.y", &pos.y, 8);
    input_float("pos.z", &pos.z, 8);

    ImGui::EndGroup();
    ImGui::Spacing();

    // rotation
    ImGui::BeginGroup();
    ImGui::Spacing();

    ImGui::Text("Rotation");
    ImGui::Spacing();

    if (input_float("axis.x", &rotation_axis.x, 8))
        rotation_axis = glm::normalize(rotation_axis + 1e-9f);
    if (input_float("axis.y", &rotation_axis.y, 8))
        rotation_axis = glm::normalize(rotation_axis + 1e-9f);
    if (input_float("axis.z", &rotation_axis.z, 8))
        rotation_axis = glm::normalize(rotation_axis + 1e-9f);

    input_float("angle", &rotation_angle, 8);

    ImGui::EndGroup();
    ImGui::Spacing();

    // scale
    ImGui::BeginGroup();
    ImGui::Spacing();

    ImGui::Text("Scale");
    ImGui::Spacing();
    const float min_scale = 1e-4f;

    if (input_float("scale.x", &scale.x, 4)) scale.x = std::max(scale.x, min_scale);
    if (input_float("scale.y", &scale.y, 4)) scale.y = std::max(scale.y, min_scale);
    if (input_float("scale.z", &scale.z, 4)) scale.z = std::max(scale.z, min_scale);

    ImGui::EndGroup();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Text("Member parameters");
    ImGui::Spacing();

    // mass
    input_float("mass (kg)", &mass, 8);
    ImGui::Spacing();

    // friction
    ImGui::SliderFloat("friction", &friction, 0.f, 1.f);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::Button("Create member") && !member_name.empty()) {
        if (builder_env->add_member(
                member_name, CUBE, pos, axis_angle_to_quat(rotation_axis, rotation_angle), scale,
                mass, friction)) {

            member_name = "";
            pos = glm::vec3(0.f);
            rotation_axis = glm::vec3(0, 1, 0);
            rotation_angle = 0.f;
            scale = glm::vec3(1.f);
            mass = 1.f;
            friction = 0.5f;

            close();
        }
    }
}

void NewMemberWindow::on_close(const std::shared_ptr<ItemFocusContext> &context) {}
void NewMemberWindow::on_focus_change(
    bool new_focus, const std::shared_ptr<ItemFocusContext> &context) {}
