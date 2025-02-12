//
// Created by samuel on 11/02/25.
//

#include <utility>

#include <imgui.h>

#include "./window.h"

ImGuiWindow::ImGuiWindow(std::string name) : name(std::move(name)), show(false) {}

void ImGuiWindow::open() { show = true; }

void ImGuiWindow::close() { show = false; }

void ImGuiWindow::render_window(const std::shared_ptr<AppContext> &context) {
    if (show) {
        if (ImGui::Begin(name.c_str(), &show)) { render_window_content(context); }
        ImGui::End();
    }
}

ImGuiWindow::~ImGuiWindow() = default;
