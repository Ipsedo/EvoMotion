//
// Created by samuel on 30/01/25.
//

#ifndef EVO_MOTION_OPENGL_WINDOW_H
#define EVO_MOTION_OPENGL_WINDOW_H

#include <memory>

#include <evo_motion_model/environment.h>
#include <evo_motion_model/robot/builder.h>
#include <evo_motion_networks/agent.h>
#include <evo_motion_view/camera.h>
#include <evo_motion_view/drawable.h>

#include "./camera.h"
#include "./context.h"
#include "./event.h"
#include "./frame_buffer.h"

class OpenGlWindow {
public:
    OpenGlWindow(std::string bar_item_name, const std::shared_ptr<Environment> &env);

    void draw_opengl(float width, float height);
    bool draw_imgui_image();

    bool is_active() const;
    std::string get_name();

protected:
    std::mt19937 rng;

    std::shared_ptr<Environment> get_env();

private:
    std::string name;

    std::unique_ptr<FrameBuffer> frame_buffer;
    std::unordered_map<std::string, std::shared_ptr<Drawable>> drawables;
    std::shared_ptr<Environment> env;

    bool active;

    std::unique_ptr<ImGuiCamera> camera;

protected:
    virtual void on_imgui_tab_begin();
    virtual void on_opengl_frame(
        float new_width, float new_height, const glm::mat4 &new_view_matrix,
        const glm::mat4 &new_proj_matrix);

    virtual std::shared_ptr<DrawableFactory>
    get_drawable_factory(const Item &item, std::mt19937 &curr_rng);

    virtual void on_hide_tab();
};

/*
 * Builder
 */

class BuilderOpenGlWindow : public OpenGlWindow {
private:
    std::shared_ptr<AppContext> context;

    std::unique_ptr<RayMouseEvent> mouse_event;

    std::shared_ptr<RobotBuilderEnvironment> builder_env;

public:
    BuilderOpenGlWindow(
        const std::shared_ptr<AppContext> &context, std::string bar_item_name,
        const std::shared_ptr<RobotBuilderEnvironment> &env);

protected:
    void on_imgui_tab_begin() override;
    void on_opengl_frame(
        float new_width, float new_height, const glm::mat4 &new_view_matrix,
        const glm::mat4 &new_proj_matrix) override;
    std::shared_ptr<DrawableFactory>
    get_drawable_factory(const Item &item, std::mt19937 &curr_rng) override;
    void on_hide_tab() override;
};

/*
 * Run
 */

class InferOpenGlWindow : public OpenGlWindow {
public:
    InferOpenGlWindow(
        const std::shared_ptr<Agent> &agent, std::string bar_item_name,
        const std::shared_ptr<Environment> &env);

protected:
    void on_opengl_frame(
        float new_width, float new_height, const glm::mat4 &new_view_matrix,
        const glm::mat4 &new_proj_matrix) override;

private:
    std::shared_ptr<Agent> agent;

    step curr_step;
};

#endif//EVO_MOTION_OPENGL_WINDOW_H