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
#include "./widget/opengl_window.h"
#include "./widget/window.h"

class ImGuiApplication final {
public:
    ImGuiApplication(const std::string &title, int width, int height);

    void render();

    bool is_close() const;

    virtual ~ImGuiApplication();

private:
    GLFWwindow *window;
    bool need_close;

    ImVec4 clear_color;

    std::shared_ptr<ItemFocusContext> context;

    std::queue<std::tuple<std::string, std::string, std::shared_ptr<RobotBuilderEnvironment>>>
        member_popup_queue;

    // window maps
    std::map<std::string, std::vector<std::shared_ptr<ImGuiWindow>>> imgui_windows;
    std::vector<std::shared_ptr<OpenGlWindow>> opengl_windows;

    ImGui::FileBrowser robot_builder_file_dialog;

    std::string popup_already_opened_robot;

    PartKind part_kind;

    GLuint vao;

    void imgui_render_toolbar();
    void imgui_render_robot_builder_file_dialog();

    void imgui_render_opengl();

    static void GLAPIENTRY message_callback(
        GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
        const GLchar *message, const void *userParam);

    std::shared_ptr<BuilderOpenGlWindow>
    create_builder_opengl_window(const std::shared_ptr<RobotBuilderEnvironment> &builder_env);

    static bool contains_window(
        const std::vector<std::shared_ptr<ImGuiWindow>> &windows, const std::string &window_name);
};

#endif//EVO_MOTION_APPLICATION_H
