//
// Created by samuel on 11/02/25.
//

#include "../window.h"

NewMemberWindow::NewMemberWindow()
    : ImGuiWindow("New member"), member_name(""), pos(0), rotation_axis(0, 1, 0),
      rotation_angle(0.f), scale(1.f), mass(1.f), friction(0.5f) {}

void NewMemberWindow::render_window_content(const std::shared_ptr<AppContext> &context) {
    member_name.resize(128);

    if (ImGui::InputText("Name", &member_name[0], member_name.size()))
        member_name = member_name.c_str();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Columns(2, nullptr, false);

    // position
    ImGui::BeginGroup();
    ImGui::Spacing();

    ImGui::Text("Position");
    ImGui::Spacing();

    ImGui::InputFloat("pos.x", &pos.x, 0.f, 0.f, "%.8f");
    ImGui::InputFloat("pos.y", &pos.y, 0.f, 0.f, "%.8f");
    ImGui::InputFloat("pos.z", &pos.z, 0.f, 0.f, "%.8f");

    ImGui::EndGroup();
    ImGui::Spacing();

    // rotation
    ImGui::BeginGroup();
    ImGui::Spacing();

    ImGui::Text("Rotation");
    ImGui::Spacing();

    if (ImGui::InputFloat("axis.x", &rotation_axis.x, 0.f, 0.f, "%.8f"))
        rotation_axis = glm::normalize(rotation_axis);
    if (ImGui::InputFloat("axis.y", &rotation_axis.y, 0.f, 0.f, "%.8f"))
        rotation_axis = glm::normalize(rotation_axis);
    if (ImGui::InputFloat("axis.z", &rotation_axis.z, 0.f, 0.f, "%.8f"))
        rotation_axis = glm::normalize(rotation_axis);
    ImGui::Spacing();
    ImGui::InputFloat("angle", &rotation_angle, 0.f, 0.f, "%.8f");

    ImGui::EndGroup();
    ImGui::Spacing();

    // scale
    ImGui::BeginGroup();
    ImGui::Spacing();

    ImGui::Text("Scale");
    ImGui::Spacing();

    if (ImGui::InputFloat("scale.x", &scale.x, 0.f, 0.f, "%.8f"))
        scale.x = std::max(scale.x, 1e-8f);
    if (ImGui::InputFloat("scale.y", &scale.y, 0.f, 0.f, "%.8f"))
        scale.y = std::max(scale.y, 1e-8f);
    if (ImGui::InputFloat("scale.z", &scale.z, 0.f, 0.f, "%.8f"))
        scale.z = std::max(scale.z, 1e-8f);

    ImGui::EndGroup();
    ImGui::Spacing();

    // next column
    ImGui::NextColumn();

    ImGui::Spacing();

    // mass
    ImGui::InputFloat("mass (kg)", &mass, 0.f, 0.f, "%.8f");
    ImGui::Spacing();

    // friction
    ImGui::DragFloat("friction", &friction, 0.01f, 0.f, 1.f);

    ImGui::Columns(1);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::Button("Create member")) {
        if (context->get_builder_env()->add_member(
                member_name, CUBE, pos, glm::angleAxis(rotation_angle, rotation_axis), scale, mass,
                friction)) {
            member_name = "";
            pos = glm::vec3(0.f);
            rotation_axis = glm::vec3(0, 1, 0);
            rotation_angle = 0.f;
            scale = glm::vec3(1.f);
            mass = 1.f;
            friction = 0.5f;
        }
    }
}
