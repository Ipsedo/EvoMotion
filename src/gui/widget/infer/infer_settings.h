//
// Created by samuel on 12/02/25.
//

#ifndef EVO_MOTION_INFER_SETTINGS_H
#define EVO_MOTION_INFER_SETTINGS_H

#include "../window.h"

class InferSettingsWindow final : public ImGuiWindow {
public:
    explicit InferSettingsWindow(
        const std::function<void(std::shared_ptr<OpenGlWindow>)> &on_start_infer);
    void render_window(const std::shared_ptr<ItemFocusContext> &context) override;

protected:
    void render_window_content(const std::shared_ptr<ItemFocusContext> &context) override;
    void on_close(const std::shared_ptr<ItemFocusContext> &context) override;
    void on_focus_change(bool new_focus, const std::shared_ptr<ItemFocusContext> &context) override;

private:
    std::function<void(std::shared_ptr<OpenGlWindow>)> on_start_infer;

    ImGui::FileBrowser robot_infer_file_dialog;
    ImGui::FileBrowser agent_infer_file_dialog;

    std::optional<std::filesystem::path> robot_json_path;
    std::optional<std::filesystem::path> agent_folder_path;
};

#endif//EVO_MOTION_INFER_SETTINGS_H
