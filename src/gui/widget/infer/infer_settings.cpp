//
// Created by samuel on 11/02/25.
//

#include "./infer_settings.h"

#include <evo_motion_networks/agents/cross_q.h>

InferSettingsWindow::InferSettingsWindow(
    const std::function<void(std::shared_ptr<OpenGlWindow>)> &on_start_infer)
    : ImGuiWindow("Infer trained agent"), on_start_infer(on_start_infer), robot_infer_file_dialog(),
      agent_infer_file_dialog(ImGuiFileBrowserFlags_SelectDirectory), robot_json_path(std::nullopt),
      agent_folder_path(std::nullopt) {

    robot_infer_file_dialog.SetTitle("Load robot JSON");
    robot_infer_file_dialog.SetTypeFilters({".json"});

    agent_infer_file_dialog.SetTitle("Load agent directory");
}

void InferSettingsWindow::render_window_content(const std::shared_ptr<ItemFocusContext> &context) {

    ImGui::Spacing();

    // Robot
    std::string loaded_json_path =
        robot_json_path.has_value() ? robot_json_path.value().string() : "No robot selected";
    loaded_json_path.resize(1024);

    if (ImGui::InputText("Load robot", &loaded_json_path[0], loaded_json_path.size()))
        robot_json_path = std::filesystem::path(loaded_json_path.c_str());

    ImGui::SameLine();
    if (ImGui::ArrowButton("Load robot button", ImGuiDir_Right)) robot_infer_file_dialog.Open();

    ImGui::Spacing();

    // Agent
    std::string loaded_agent_path =
        agent_folder_path.has_value() ? agent_folder_path.value().string() : "No agent selected";
    loaded_agent_path.resize(1024);

    if (ImGui::InputText("Load agent", &loaded_agent_path[0], loaded_agent_path.size()))
        agent_folder_path = std::filesystem::path(loaded_agent_path.c_str());

    ImGui::SameLine();
    if (ImGui::ArrowButton("Load agent button", ImGuiDir_Right)) agent_infer_file_dialog.Open();

    ImGui::Spacing();
    ImGui::Separator();

    if (ImGui::Button("Start inference") && agent_folder_path.has_value()
        && robot_json_path.has_value()) {
        const auto env = get_environment_factory("robot_walk", {})->get_env(4, 1234);
        const auto agent = std::make_shared<CrossQAgent>(
            12345, env->get_state_space(), env->get_action_space(), 256, 1024, 128, 1, 3e-4f, 0.99f,
            1, 2);
        agent->load(agent_folder_path.value());// TODO check if loaded successfully
        agent->to(torch::kCPU);

        const auto now = std::chrono::system_clock::now().time_since_epoch();
        const long id = std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
        const auto gl_window =
            std::make_shared<InferOpenGlWindow>(agent, "Infer " + std::to_string(id), env);

        close();

        on_start_infer(gl_window);
    }
}

void InferSettingsWindow::render_window(const std::shared_ptr<ItemFocusContext> &context) {
    // robot JSON file dialog
    robot_infer_file_dialog.Display();
    if (robot_infer_file_dialog.HasSelected()) {
        robot_json_path = robot_infer_file_dialog.GetSelected();
        robot_infer_file_dialog.ClearSelected();
    }

    // agent state dict folder dialog
    agent_infer_file_dialog.Display();
    if (agent_infer_file_dialog.HasSelected()) {
        agent_folder_path = agent_infer_file_dialog.GetSelected();
        agent_infer_file_dialog.ClearSelected();
    }

    ImGuiWindow::render_window(context);
}

void InferSettingsWindow::on_close(const std::shared_ptr<ItemFocusContext> &context) {}
void InferSettingsWindow::on_focus_change(
    bool new_focus, const std::shared_ptr<ItemFocusContext> &context) {}
