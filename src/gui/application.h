//
// Created by samuel on 28/01/25.
//

#ifndef EVO_MOTION_APPLICATION_H
#define EVO_MOTION_APPLICATION_H

#include <optional>
#include <random>
#include <string>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

// clang-format off
#include <imfilebrowser.h>
// clang-format on

#include <evo_motion_model/robot/builder.h>
#include <evo_motion_view/camera.h>
#include <evo_motion_view/factory.h>

#include "./camera.h"
#include "./context.h"
#include "./frame_buffer.h"
#include "./opengl_window.h"

class ImGuiApplication {
public:
    ImGuiApplication(const std::string &title, int width, int height);

    void render();

    bool is_close() const;

    virtual ~ImGuiApplication();

private:
    GLFWwindow *window;
    bool need_close;

    ImVec4 clear_color;

    bool show_member_settings_window;
    bool show_construct_tools_window;
    bool show_manage_trainings_window;
    bool show_new_training_window;
    bool show_robot_info_window;
    bool show_infer_window;

    std::shared_ptr<AppContext> context;

    std::vector<std::shared_ptr<OpenGlWindow>> opengl_windows;

    ImGui::FileBrowser robot_builder_file_dialog;

    ImGui::FileBrowser robot_infer_file_dialog;
    ImGui::FileBrowser agent_infer_file_dialog;

    ImVec2 opengl_render_size;

    std::string popup_already_opened_robot;

    GLuint vao;

    void imgui_render_toolbar();

    void imgui_render_robot_builder_file_dialog();
    void imgui_render_member_settings();
    void imgui_render_construct_tools();
    void imgui_render_robot_info();

    void imgui_render_opengl();

    void imgui_render_new_training();
    void imgui_render_manage_trainings();

    void imgui_render_robot_infer_file_dialog();
    void imgui_render_agent_infer_file_dialog();
    void imgui_render_robot_infer();

    static void GLAPIENTRY message_callback(
        const GLenum source, const GLenum type, const GLuint id, const GLenum severity,
        const GLsizei length, const GLchar *message, const void *userParam);
};

#endif//EVO_MOTION_APPLICATION_H
