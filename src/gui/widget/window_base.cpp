//
// Created by samuel on 11/02/25.
//

#include <utility>

#include <imgui.h>

#include "./window.h"

ImGuiWindow::ImGuiWindow(std::string name) : name(std::move(name)), show(true), focus(false) {}

void ImGuiWindow::close() { show = false; }

bool ImGuiWindow::is_closed() const { return !show; }

std::optional<std::shared_ptr<ImGuiWindow>> ImGuiWindow::pop_child() { return std::nullopt; }

void ImGuiWindow::render_window(const std::shared_ptr<ItemFocusContext> &context) {
    if (show) {
        if (ImGui::Begin(name.c_str(), &show)) {
            bool new_focus = ImGui::IsWindowFocused();
            if (new_focus != focus) on_focus_change(new_focus, context);
            focus = new_focus;

            render_window_content(context);
        }
        ImGui::End();
    }
    if (!show) on_close(context);
}

ImGuiWindow::~ImGuiWindow() = default;
