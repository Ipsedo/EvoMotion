//
// Created by samuel on 12/02/25.
//

#include "./member_popup.h"

#include "../robot_info.h"
#include "./duplicate_member.h"
#include "./member_construct_tools.h"
#include "./member_settings.h"
#include "./new_member.h"

FocusMemberPopUpWindow::FocusMemberPopUpWindow(
    const std::string &member_name, const std::shared_ptr<RobotBuilderEnvironment> &builder_env)
    : PopUpWindow(member_name), member_name(member_name), builder_env(builder_env),
      children(std::nullopt) {}

void FocusMemberPopUpWindow::render_popup_content(
    const std::shared_ptr<ItemFocusContext> &context) {
    if (ImGui::MenuItem("Setting")) {
        context->release_focus(member_name);
        children = std::make_shared<MemberSettingsWindow>(member_name, builder_env);
        close();
    }
    if (ImGui::MenuItem("Construct tools")) {
        context->release_focus(member_name);
        children = std::make_shared<MemberConstructToolsWindow>(member_name, builder_env);
        close();
    }
    if (ImGui::MenuItem("Duplicate")) {
        context->release_focus(member_name);
        children = std::make_shared<DuplicateGroupWindow>(member_name, builder_env);
        close();
    }
}

void FocusMemberPopUpWindow::on_close(const std::shared_ptr<ItemFocusContext> &context) {
    context->release_focus(member_name);
}

void FocusMemberPopUpWindow::on_focus_change(
    const bool new_focus, const std::shared_ptr<ItemFocusContext> &context) {
    if (new_focus) context->focus_black(member_name);
    else context->release_focus(member_name);
}

std::optional<std::shared_ptr<ImGuiWindow>> FocusMemberPopUpWindow::pop_child() {
    if (children.has_value()) {
        auto return_value = children.value();
        children = std::nullopt;
        return return_value;
    }
    return children;
}

bool FocusMemberPopUpWindow::need_close() { return !builder_env->member_exists(member_name); }

/*
 * No focus
 */

NoFocusMemberPopUpWindow::NoFocusMemberPopUpWindow(
    const std::shared_ptr<RobotBuilderEnvironment> &builder_env)
    : PopUpWindow("Member"), builder_env(builder_env), children(std::nullopt) {}

void NoFocusMemberPopUpWindow::render_popup_content(
    const std::shared_ptr<ItemFocusContext> &context) {

    if (ImGui::MenuItem("New")) {
        children = std::make_shared<NewMemberWindow>(builder_env);
        close();
    }

    ImGui::Separator();

    if (ImGui::MenuItem("Robot info")) {
        children = std::make_shared<RobotInfoWindow>(builder_env);
        close();
    }
}

void NoFocusMemberPopUpWindow::on_close(const std::shared_ptr<ItemFocusContext> &context) {}

void NoFocusMemberPopUpWindow::on_focus_change(
    bool new_focus, const std::shared_ptr<ItemFocusContext> &context) {}

std::optional<std::shared_ptr<ImGuiWindow>> NoFocusMemberPopUpWindow::pop_child() {
    if (children.has_value()) {
        const auto return_value = children.value();
        children = std::nullopt;
        return return_value;
    }
    return children;
}

bool NoFocusMemberPopUpWindow::need_close() { return false; }
