//
// Created by samuel on 28/01/25.
//

#ifndef EVO_MOTION_GUI_RENDERER_H
#define EVO_MOTION_GUI_RENDERER_H

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
#include "./frame_buffer.h"
#include "./robot_tab.h"

class ImGuiRenderer {
public:
    ImGuiRenderer(const std::string &title, int width, int height);

    void draw();

    bool is_close() const;

    virtual ~ImGuiRenderer();

private:
    GLFWwindow *window;
    bool need_close;

    ImVec4 clear_color;

    bool show_member_window;
    bool show_construct_tools_window;
    bool show_training_window;
    bool show_robot_builder_window;

    std::shared_ptr<ImGuiCamera> builder_camera;
    std::vector<std::shared_ptr<RobotBuilderEnvironment>> loaded_robots;
    int curr_loaded_robot_index;
    std::optional<std::string> member_focus;

    ImGui::FileBrowser robot_file_dialog;

    std::map<std::string, std::shared_ptr<Drawable>> drawables;

    std::mt19937 rng;
    std::uniform_real_distribution<float> rd_uni;

    ImVec2 opengl_render_size;

    std::unique_ptr<FrameBuffer> frame_buffer;

    void init_robot();

    void opengl_render_robot(
        const std::shared_ptr<Environment> &env, const std::shared_ptr<Camera> &camera);

    void imgui_render_toolbar();
    void imgui_render_construct_tools();
    void imgui_render_robot_construct();
    void imgui_render_file_dialog();
};

#endif//EVO_MOTION_GUI_RENDERER_H
