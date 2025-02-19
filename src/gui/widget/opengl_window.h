//
// Created by samuel on 30/01/25.
//

#ifndef EVO_MOTION_OPENGL_WINDOW_H
#define EVO_MOTION_OPENGL_WINDOW_H

#include <memory>

#include <evo_motion_model/environment.h>
#include <evo_motion_model/robot/builder.h>
#include <evo_motion_networks/agent.h>
#include <evo_motion_view/drawable.h>
#include <evo_motion_view/frame_buffer.h>

#include "../camera.h"
#include "../context.h"
#include "../event.h"

class OpenGlWindow {
public:
    virtual ~OpenGlWindow() = default;

    OpenGlWindow(std::string bar_item_name, const std::shared_ptr<Environment> &env);

    void draw_opengl(float width, float height);
    void draw_imgui_image();

    bool is_active() const;
    bool is_opened() const;
    std::string get_name();

    std::tuple<float, float> get_window_start_pos();
    std::tuple<float, float> get_width_height();

    void rename_drawable(const std::string &old_name, const std::string &new_name);
    void add_item(const std::shared_ptr<NoShapeItem> &no_shape_item);
    void remove_item(const std::shared_ptr<NoShapeItem> &no_shape_item);

    glm::mat4 get_view_matrix();
    glm::mat4 get_projection_matrix();

protected:
    std::mt19937 rng;

    std::shared_ptr<Environment> get_env();

private:
    std::string name;

    std::unique_ptr<FrameBuffer> frame_buffer;
    std::unordered_map<std::string, std::shared_ptr<Drawable>> drawables;
    std::shared_ptr<Environment> env;
    std::vector<std::shared_ptr<NoShapeItem>> no_shape_items;

    bool active;
    bool opened;

    std::unique_ptr<ImGuiCamera> camera;

    glm::mat4 view_matrix;
    glm::mat4 projection_matrix;

    float window_start_x;
    float window_start_y;
    float window_width;
    float window_height;

protected:
    virtual void on_imgui_tab_begin() = 0;
    virtual void on_opengl_frame(
        float new_width, float new_height, const glm::mat4 &new_view_matrix,
        const glm::mat4 &new_proj_matrix) = 0;

    virtual std::shared_ptr<DrawableFactory>
    get_drawable_factory(const std::shared_ptr<ShapeItem> &item, std::mt19937 &curr_rng);

    virtual std::shared_ptr<DrawableFactory>
    get_drawable_factory(const std::shared_ptr<NoShapeItem> &item);
};

/*
 * Builder
 */

enum PartKind { MEMBER, CONSTRAINT, MUSCLE };

class BuilderOpenGlWindow final : public OpenGlWindow {
private:
    std::shared_ptr<ItemFocusContext> context;

    std::unique_ptr<RayMouseEvent> mouse_event;

    std::shared_ptr<RobotBuilderEnvironment> builder_env;

    std::function<void(
        std::string, std::optional<std::string>, std::shared_ptr<RobotBuilderEnvironment>)>
        on_member_focused;
    std::function<void(
        std::string, std::optional<std::string>, std::shared_ptr<RobotBuilderEnvironment>)>
        on_constraint_focused;

    std::function<PartKind()> get_part_type;

    std::shared_ptr<Drawable> cube_grid;

public:
    BuilderOpenGlWindow(
        const std::shared_ptr<ItemFocusContext> &context, std::string bar_item_name,
        const std::shared_ptr<RobotBuilderEnvironment> &env,
        const std::function<
            void(std::string, std::optional<std::string>, std::shared_ptr<RobotBuilderEnvironment>)>
            &on_member_focused,
        const std::function<
            void(std::string, std::optional<std::string>, std::shared_ptr<RobotBuilderEnvironment>)>
            &on_constraint_focused,
        const std::function<PartKind()> &get_part_type);

protected:
    void on_imgui_tab_begin() override;
    void on_opengl_frame(
        float new_width, float new_height, const glm::mat4 &new_view_matrix,
        const glm::mat4 &new_proj_matrix) override;
    std::shared_ptr<DrawableFactory>
    get_drawable_factory(const std::shared_ptr<ShapeItem> &item, std::mt19937 &curr_rng) override;
};

/*
 * Run
 */

class InferOpenGlWindow final : public OpenGlWindow {
public:
    InferOpenGlWindow(
        const std::shared_ptr<Agent> &agent, std::string bar_item_name,
        const std::shared_ptr<Environment> &env);

protected:
    void on_opengl_frame(
        float new_width, float new_height, const glm::mat4 &new_view_matrix,
        const glm::mat4 &new_proj_matrix) override;
    void on_imgui_tab_begin() override;

private:
    std::shared_ptr<Agent> agent;

    step curr_step;
};

#endif//EVO_MOTION_OPENGL_WINDOW_H