//
// Created by samuel on 11/02/25.
//

#include "./window.h"

#include <utility>

#include <imgui.h>

ImGuiWindow::ImGuiWindow(std::string name) : name(std::move(name)), show(true), focus(false) {}

void ImGuiWindow::close() { show = false; }

bool ImGuiWindow::is_closed() const { return !show; }

std::optional<std::shared_ptr<ImGuiWindow>> ImGuiWindow::pop_child() { return std::nullopt; }

std::string ImGuiWindow::get_name() const { return name; }

void ImGuiWindow::render_window(const std::shared_ptr<ItemFocusContext> &context) {
    if (show) {
        if (ImGui::Begin(name.c_str(), &show)) {
            if (ImGui::IsWindowFocused() != focus) {
                focus = !focus;
                on_focus_change(focus, context);
            }

            render_window_content(context);

            ImGui::End();
        }
    }
    if (!show) on_close(context);
}

ImGuiWindow::~ImGuiWindow() = default;
