//
// Created by samuel on 13/02/25.
//

#include "./constraint_popup.h"

#include "../robot_info.h"
#include "./constraint_construct_tools.h"
#include "./constraint_settings.h"
#include "./new_constraint.h"

FocusConstraintPopUpWindow::FocusConstraintPopUpWindow(
    const std::string &constraint_name, const std::shared_ptr<RobotBuilderEnvironment> &builder_env)
    : PopUpWindow(constraint_name), constraint_name(constraint_name), builder_env(builder_env),
      children(std::nullopt) {

    const auto [p_name, c_name] = builder_env->get_constraint_members(constraint_name);
    parent_item = p_name;
    child_item = c_name;
}

void FocusConstraintPopUpWindow::render_popup_content(
    const std::shared_ptr<ItemFocusContext> &context) {
    if (ImGui::MenuItem("Settings")) {
        clear_focus(context);

        switch (builder_env->get_constraint_type(constraint_name)) {
            case HINGE:
                children =
                    std::make_shared<HingeConstraintSettingsWindow>(constraint_name, builder_env);
                break;
            case FIXED:
                children =
                    std::make_shared<FixedConstraintSettingsWindow>(constraint_name, builder_env);
                break;
        }

        close();
    }

    if (ImGui::MenuItem("Construct tools")) {
        switch (builder_env->get_constraint_type(constraint_name)) {
            case HINGE:
                children =
                    std::make_shared<HingeConstructToolsWindow>(constraint_name, builder_env);
                break;
            case FIXED: break;
        }

        close();
    }
}

void FocusConstraintPopUpWindow::on_close(const std::shared_ptr<ItemFocusContext> &context) {
    clear_focus(context);
}

void FocusConstraintPopUpWindow::on_focus_change(
    const bool new_focus, const std::shared_ptr<ItemFocusContext> &context) {
    if (new_focus) add_focus(context);
    else clear_focus(context);
}

void FocusConstraintPopUpWindow::add_focus(const std::shared_ptr<ItemFocusContext> &context) const {
    context->focus_black(constraint_name);

    context->focus_grey(parent_item);
    context->focus_grey(child_item);
}

void FocusConstraintPopUpWindow::clear_focus(
    const std::shared_ptr<ItemFocusContext> &context) const {
    context->release_focus(constraint_name);

    context->release_focus(parent_item);
    context->release_focus(child_item);
}

std::optional<std::shared_ptr<ImGuiWindow>> FocusConstraintPopUpWindow::pop_child() {
    if (children.has_value()) {
        const auto return_value = children;
        children = std::nullopt;
        return return_value;
    }
    return children;
}
bool FocusConstraintPopUpWindow::need_close() {
    return !builder_env->constraint_exists(constraint_name);
}

/*
 * No focus
 */

NoFocusConstraintPopUpWindow::NoFocusConstraintPopUpWindow(
    const std::shared_ptr<RobotBuilderEnvironment> &builder_env)
    : PopUpWindow("Constraint"), builder_env(builder_env), children(std::nullopt) {}

std::optional<std::shared_ptr<ImGuiWindow>> NoFocusConstraintPopUpWindow::pop_child() {
    if (children.has_value()) {
        auto return_value = children.value();
        children = std::nullopt;
        return return_value;
    }
    return children;
}

void NoFocusConstraintPopUpWindow::on_close(const std::shared_ptr<ItemFocusContext> &context) {}

void NoFocusConstraintPopUpWindow::on_focus_change(
    bool new_focus, const std::shared_ptr<ItemFocusContext> &context) {}

void NoFocusConstraintPopUpWindow::render_popup_content(
    const std::shared_ptr<ItemFocusContext> &context) {

    if (ImGui::MenuItem("New hinge")) {
        children = std::make_shared<NewHingeConstraintWindow>(builder_env);
        close();
    }
    if (ImGui::MenuItem("New fixed")) {
        children = std::make_shared<NewFixedConstraintWindow>(builder_env);
        close();
    }

    ImGui::Separator();

    if (ImGui::MenuItem("Robot info")) {
        children = std::make_shared<RobotInfoWindow>(builder_env);
        close();
    }
}

bool NoFocusConstraintPopUpWindow::need_close() { return false; }
