//
// Created by samuel on 11/02/25.
//

#include "./member_construct_tools.h"

#include "../construct/rotate.h"

MemberConstructToolsWindow::MemberConstructToolsWindow()
    : ImGuiWindow("Construct tools - Member") {}

void MemberConstructToolsWindow::render_window_content(
    const std::shared_ptr<ItemFocusContext> &context) {}

void MemberConstructToolsWindow::on_close(const std::shared_ptr<ItemFocusContext> &context) {}
void MemberConstructToolsWindow::on_focus_change(
    bool new_focus, const std::shared_ptr<ItemFocusContext> &context) {}
