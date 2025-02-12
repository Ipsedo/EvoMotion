//
// Created by samuel on 12/02/25.
//

#include "./member_popup.h"

#include "./member_construct_tools.h"
#include "./member_settings.h"

MemberMenuWindow::MemberMenuWindow(
    const std::string &member_name, const std::shared_ptr<RobotBuilderEnvironment> &builder_env)
    : ImGuiWindow(member_name), first_open(true), member_name(member_name),
      builder_env(builder_env), children(std::nullopt) {}

void MemberMenuWindow::render_window_content(const std::shared_ptr<ItemFocusContext> &context) {
    if (ImGui::MenuItem("Setting")) {
        context->release_focus(member_name);
        children = std::make_shared<MemberSettingsWindow>(member_name, builder_env);
        close();
    }
    if (ImGui::MenuItem("Construct tools")) {
        context->release_focus(member_name);
        children = std::make_shared<MemberConstructToolsWindow>();
        close();
    }
}

void MemberMenuWindow::on_close(const std::shared_ptr<ItemFocusContext> &context) {
    context->release_focus(member_name);
}

void MemberMenuWindow::on_focus_change(
    bool new_focus, const std::shared_ptr<ItemFocusContext> &context) {
    if (new_focus) context->focus_black(member_name);
    else context->release_focus(member_name);
}

std::optional<std::shared_ptr<ImGuiWindow>> MemberMenuWindow::pop_child() {
    if (children.has_value()) {
        auto return_value = children.value();
        children = std::nullopt;
        return return_value;
    }
    return children;
}

void MemberMenuWindow::render_window(const std::shared_ptr<ItemFocusContext> &context) {
    if (first_open) {
        const auto mouse_pos = ImGui::GetMousePos();
        ImGui::SetNextWindowPos(mouse_pos);
        first_open = false;
    }
    ImGuiWindow::render_window(context);
}
