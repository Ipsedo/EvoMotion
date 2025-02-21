//
// Created by samuel on 11/02/25.
//

#include "./constraint_construct_tools.h"

HingeConstructToolsWindow::HingeConstructToolsWindow(
    const std::string &constraint_name, const std::shared_ptr<RobotBuilderEnvironment> &builder_env)
    : ConstructToolsWindow(
          "Construct tools - Constraint \"" + constraint_name + "\"", true, true, false),
      constraint_name(constraint_name), builder_env(builder_env) {}

void HingeConstructToolsWindow::on_update_pos(const glm::vec3 &pos_delta) {
    const auto [pos, axis, limit_min, limit_max] =
        builder_env->get_constraint_hinge_info(constraint_name);
    builder_env->update_hinge_constraint(constraint_name, pos + pos_delta);
}

void HingeConstructToolsWindow::on_update_rot(const glm::quat &rot_delta) {
    const auto [pos, rot, scale] = builder_env->get_constraint_transform(constraint_name);

    const glm::vec3 axis(0, 0, 1);
    builder_env->update_hinge_constraint(constraint_name, std::nullopt, rot_delta * rot * axis);
}

void HingeConstructToolsWindow::on_update_scale(const glm::vec3 &scale_delta) {}

std::tuple<glm::vec3, glm::quat, glm::vec3>
HingeConstructToolsWindow::get_construct_item_model_matrix() {
    return builder_env->get_constraint_transform(constraint_name);
}

void HingeConstructToolsWindow::add_focus(const std::shared_ptr<ItemFocusContext> &context) {
    context->focus_black(constraint_name);

    const auto [parent_name, child_name] = builder_env->get_constraint_members(constraint_name);
    context->focus_grey(parent_name);
    context->focus_grey(child_name);
}

void HingeConstructToolsWindow::clear_focus(const std::shared_ptr<ItemFocusContext> &context) {
    context->release_focus(constraint_name);

    const auto [parent_name, child_name] = builder_env->get_constraint_members(constraint_name);
    context->release_focus(parent_name);
    context->release_focus(child_name);
}

bool HingeConstructToolsWindow::need_close() {
    return !builder_env->constraint_exists(constraint_name);
}
