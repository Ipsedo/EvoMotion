//
// Created by samuel on 13/02/25.
//

#include "./constraint_popup.h"

#include "./constraint_settings.h"

ConstraintMenuWindow::ConstraintMenuWindow(
    const std::string &constraint_name, const std::shared_ptr<RobotBuilderEnvironment> &builder_env)
    : PopUpWindow(constraint_name), constraint_name(constraint_name), builder_env(builder_env),
      children(std::nullopt) {

    const auto [p_name, c_name] = builder_env->get_constraint_members(constraint_name);
    parent_item = p_name;
    child_item = c_name;
}

void ConstraintMenuWindow::render_window_content(const std::shared_ptr<ItemFocusContext> &context) {
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
        // TODO
    }

    PopUpWindow::render_window_content(context);
}

void ConstraintMenuWindow::on_close(const std::shared_ptr<ItemFocusContext> &context) {
    clear_focus(context);
}

void ConstraintMenuWindow::on_focus_change(
    bool new_focus, const std::shared_ptr<ItemFocusContext> &context) {
    if (new_focus) add_focus(context);
    else clear_focus(context);
}

void ConstraintMenuWindow::add_focus(const std::shared_ptr<ItemFocusContext> &context) {
    context->focus_black(constraint_name);

    context->focus_white(parent_item);
    context->focus_white(child_item);
}

void ConstraintMenuWindow::clear_focus(const std::shared_ptr<ItemFocusContext> &context) {
    context->release_focus(constraint_name);

    context->release_focus(parent_item);
    context->release_focus(child_item);
}

std::optional<std::shared_ptr<ImGuiWindow>> ConstraintMenuWindow::pop_child() {
    if (children.has_value()) {
        const auto return_value = children;
        children = std::nullopt;
        return return_value;
    }
    return children;
}
