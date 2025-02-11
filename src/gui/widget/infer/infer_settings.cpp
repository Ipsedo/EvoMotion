//
// Created by samuel on 11/02/25.
//

#include <evo_motion_networks/agents/cross_q.h>

#include "../window.h"

InferSettingsWindow::InferSettingsWindow(
    const std::function<void(std::shared_ptr<OpenGlWindow>)> &on_start_infer)
    : ImGuiWindow("Infer trained agent"), on_start_infer(on_start_infer), robot_infer_file_dialog(),
      agent_infer_file_dialog(ImGuiFileBrowserFlags_SelectDirectory) {

    robot_infer_file_dialog.SetTitle("Load robot JSON");
    robot_infer_file_dialog.SetTypeFilters({".json"});

    agent_infer_file_dialog.SetTitle("Load agent directory");
}

void InferSettingsWindow::render_window_content(const std::shared_ptr<AppContext> &context) {

    ImGui::Spacing();

    // Robot
    std::string loaded_json_path = context->is_robot_infer_json_path_selected()
                                       ? context->get_robot_infer_json_path().string()
                                       : "No robot selected";
    loaded_json_path.resize(1024);

    if (ImGui::InputText("Load robot", &loaded_json_path[0], loaded_json_path.size()))
        context->set_robot_infer_json_path(std::filesystem::path(loaded_json_path.c_str()));

    ImGui::SameLine();
    if (ImGui::ArrowButton("Load robot button", ImGuiDir_Right)) robot_infer_file_dialog.Open();

    ImGui::Spacing();

    // Agent
    std::string loaded_agent_path = context->is_agent_infer_path_selected()
                                        ? context->get_agent_infer_path().string()
                                        : "No agent selected";
    loaded_agent_path.resize(1024);

    if (ImGui::InputText("Load agent", &loaded_agent_path[0], loaded_agent_path.size()))
        context->set_agent_infer_path(std::filesystem::path(loaded_agent_path.c_str()));

    ImGui::SameLine();
    if (ImGui::ArrowButton("Load agent button", ImGuiDir_Right)) agent_infer_file_dialog.Open();

    ImGui::Spacing();
    ImGui::Separator();

    if (ImGui::Button("Start inference") && context->is_agent_infer_path_selected()
        && context->is_robot_infer_json_path_selected()) {
        const auto env = get_environment_factory("robot_walk", {})->get_env(4, 1234);
        const auto agent = std::make_shared<CrossQAgent>(
            12345, env->get_state_space(), env->get_action_space(), 256, 1024, 128, 1, 3e-4f, 0.99f,
            1, 2);
        agent->load(context->get_agent_infer_path());// TODO check if loaded successfully
        agent->to(torch::kCPU);

        auto now = std::chrono::system_clock::now().time_since_epoch();
        int id = std::chrono::duration_cast<std::chrono::seconds>(now).count();
        auto gl_window =
            std::make_shared<InferOpenGlWindow>(agent, "Infer " + std::to_string(id), env);

        close();
        context->release_agent_infer_path();
        context->release_robot_infer_json_path();

        on_start_infer(gl_window);
    }
}

void InferSettingsWindow::render_window(const std::shared_ptr<AppContext> &context) {
    // robot JSON file dialog
    robot_infer_file_dialog.Display();
    if (robot_infer_file_dialog.HasSelected()) {
        context->set_robot_infer_json_path(robot_infer_file_dialog.GetSelected());
        robot_infer_file_dialog.ClearSelected();
    }

    // agent state dict folder dialog
    agent_infer_file_dialog.Display();
    if (agent_infer_file_dialog.HasSelected()) {
        context->set_agent_infer_path(agent_infer_file_dialog.GetSelected());
        agent_infer_file_dialog.ClearSelected();
    }

    ImGuiWindow::render_window(context);
}
