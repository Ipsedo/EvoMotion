//
// Created by samuel on 13/02/25.
//

#include "./popup.h"

PopUpWindow::PopUpWindow(const std::string &popup_name)
    : ImGuiWindow(popup_name), name(popup_name), first_open(true) {}

void PopUpWindow::render_window(
    const std::shared_ptr<ItemFocusContext> &context,
    const std::shared_ptr<OpenGlWindow> &gl_window) {
    if (first_open) {
        const auto mouse_pos = ImGui::GetMousePos();
        ImGui::SetNextWindowPos(mouse_pos);
        first_open = false;
    }
    ImGuiWindow::render_window(context, gl_window);
}

void PopUpWindow::render_window_content(
    const std::shared_ptr<ItemFocusContext> &context,
    const std::shared_ptr<OpenGlWindow> &gl_window) {
    render_popup_content(context);
    if (!ImGui::IsWindowFocused()) close();
}

ImVec2 PopUpWindow::get_min_size() { return ImVec2(0, 0); }
