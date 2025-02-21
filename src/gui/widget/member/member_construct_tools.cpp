//
// Created by samuel on 11/02/25.
//

#include "./member_construct_tools.h"

#include <glm/gtc/matrix_transform.hpp>

MemberConstructToolsWindow::MemberConstructToolsWindow(
    const std::string &member_name, const std::shared_ptr<RobotBuilderEnvironment> &builder_env)
    : ConstructToolsWindow("Construct tools of \"" + member_name + "\"", true, true, true),
      member_name(member_name), builder_env(builder_env) {}

void MemberConstructToolsWindow::on_update_pos(const glm::vec3 &pos_delta) {
    const auto [pos, rot, scale] = builder_env->get_member_transform(member_name);
    builder_env->update_member(member_name, pos + pos_delta);
}

void MemberConstructToolsWindow::on_update_rot(const glm::quat &rot_delta) {
    const auto [pos, rot, scale] = builder_env->get_member_transform(member_name);
    builder_env->update_member(member_name, std::nullopt, rot_delta * rot);
}

void MemberConstructToolsWindow::on_update_scale(const glm::vec3 &scale_delta) {
    const auto [pos, rot, scale] = builder_env->get_member_transform(member_name);
    builder_env->update_member(member_name, std::nullopt, std::nullopt, scale + scale_delta);
}

std::tuple<glm::vec3, glm::quat, glm::vec3>
MemberConstructToolsWindow::get_construct_item_model_matrix() {
    return builder_env->get_member_transform(member_name);
}

void MemberConstructToolsWindow::add_focus(const std::shared_ptr<ItemFocusContext> &context) {
    context->focus_black(member_name);
}

void MemberConstructToolsWindow::clear_focus(const std::shared_ptr<ItemFocusContext> &context) {
    context->release_focus(member_name);
}

bool MemberConstructToolsWindow::need_close() { return !builder_env->member_exists(member_name); }
