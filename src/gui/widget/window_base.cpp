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

std::string ImGuiWindow::get_name() const { return name; }

void ImGuiWindow::render_window(const std::shared_ptr<ItemFocusContext> &context) {
    if (show) {
        ImVec2 title_size = ImGui::CalcTextSize(name.c_str());
        ImGui::SetNextWindowSizeConstraints(
            ImVec2(
                ImGui::GetTextLineHeight() * 4.f + title_size.x
                    + ImGui::GetStyle().FramePadding.x * 2.f,
                0),
            ImVec2(FLT_MAX, FLT_MAX));
        if (ImGui::Begin(name.c_str(), &show, ImGuiWindowFlags_AlwaysAutoResize)) {

            if (ImGui::IsWindowFocused() != focus) {
                focus = !focus;
                on_focus_change(focus, context);
            }

            render_window_content(context);
        }
        ImGui::End();
    }
    if (!show) on_close(context);
}

ImGuiWindow::~ImGuiWindow() = default;
