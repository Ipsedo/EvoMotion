//
// Created by samuel on 28/01/25.
//

#ifndef EVO_MOTION_RENDERER_H
#define EVO_MOTION_RENDERER_H

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
#include <evo_motion_view/factory.h>

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

    bool show_construct_tools;
    bool show_training;
    bool show_construct_robot;

    std::vector<std::shared_ptr<RobotBuilderEnvironment>> loaded_robots;
    int curr_loaded_robot_index;

    ImGui::FileBrowser robot_file_dialog;

    std::map<std::string, std::shared_ptr<Drawable>> drawables;

    std::mt19937 rng;
    std::uniform_real_distribution<float> rd_uni;

    ImVec2 opengl_render_size;

    std::unique_ptr<FrameBuffer> frame_buffer;

    void init_drawables();

    void render_opengl_robot();
};

#endif//EVO_MOTION_RENDERER_H
