//
// Created by samuel on 30/01/25.
//

#ifndef EVO_MOTION_OPENGL_WINDOW_H
#define EVO_MOTION_OPENGL_WINDOW_H

#include <memory>

#include <evo_motion_model/environment.h>
#include <evo_motion_model/robot/builder.h>
#include <evo_motion_view/camera.h>
#include <evo_motion_view/drawable.h>

#include "./camera.h"
#include "./frame_buffer.h"

class OpenGlWindow final {
public:
    OpenGlWindow(const std::string &bar_item_name, const std::shared_ptr<Environment> &env);

    void draw_opengl(float width, float height);
    bool draw_imgui_image();

    bool is_active();

    std::shared_ptr<Environment> get_env();

private:
    std::string name;

    std::unique_ptr<FrameBuffer> frame_buffer;
    std::unordered_map<std::string, std::shared_ptr<Drawable>> drawables;
    std::shared_ptr<Environment> env;

    std::mt19937 rng;
    std::uniform_real_distribution<float> rd_uni;

    bool active;

    std::shared_ptr<ImGuiCamera> camera;
};

#endif//EVO_MOTION_OPENGL_WINDOW_H
