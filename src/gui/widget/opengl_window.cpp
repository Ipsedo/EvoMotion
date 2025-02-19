//
// Created by samuel on 30/01/25.
//

#include "./opengl_window.h"

#include <utility>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>

#include <evo_motion_view/factory.h>

OpenGlWindow::OpenGlWindow(std::string bar_item_name, const std::shared_ptr<Environment> &env)
    : rng(std::chrono::duration_cast<std::chrono::microseconds>(
              std::chrono::system_clock::now().time_since_epoch())
              .count()),
      name(std::move(bar_item_name)), frame_buffer(std::make_unique<FrameBuffer>(1920, 1080)),
      drawables(), env(env), no_shape_items(), active(true), opened(true),
      camera(std::make_unique<ImGuiCamera>([env]() {
          if (const auto track_item = env->get_camera_track_item(); track_item.has_value())
              return glm::vec3(track_item.value()->model_matrix_without_scale()[3]);
          return glm::vec3(0.f);
      })),
      view_matrix(1.f), projection_matrix(1.f), window_start_x(0), window_start_y(0),
      window_width(1920), window_height(1080) {}

std::string OpenGlWindow::get_name() { return name; }

void OpenGlWindow::rename_drawable(const std::string &old_name, const std::string &new_name) {
    auto n = drawables.extract(old_name);
    n.key() = new_name;
    drawables.insert(std::move(n));
}

void OpenGlWindow::add_item(const std::shared_ptr<NoShapeItem> &no_shape_item) {
    no_shape_items.push_back(no_shape_item);
}

void OpenGlWindow::remove_item(const std::shared_ptr<NoShapeItem> &no_shape_item) {
    std::erase_if(no_shape_items, [no_shape_item](const auto &i) {
        return i->get_name() == no_shape_item->get_name();
    });
    if (drawables.contains(no_shape_item->get_name())) drawables.erase(no_shape_item->get_name());
}

void OpenGlWindow::draw_opengl(const float width, const float height) {
    window_width = width;
    window_height = height;

    view_matrix = glm::lookAt(camera->pos(), camera->look(), camera->up());
    projection_matrix = glm::frustum(-1.f, 1.f, -height / width, height / width, 1.f, 200.f);

    const glm::vec3 light_pos(0, 20, 0);
    const glm::vec3 cam_pos = camera->pos();

    frame_buffer->rescale_frame_buffer(width, height);
    frame_buffer->bind();

    glViewport(0, 0, static_cast<int>(width), static_cast<int>(height));
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    on_opengl_frame(width, height, view_matrix, projection_matrix);

    for (const auto &i: env->get_draw_items()) {
        if (!drawables.contains(i->get_name()))
            drawables[i->get_name()] = get_drawable_factory(i, rng)->create_drawable();

        drawables[i->get_name()]->draw(
            projection_matrix, view_matrix, i->model_matrix(), light_pos, cam_pos);
    }

    for (const auto &i: no_shape_items) {
        if (!drawables.contains(i->get_name()))
            drawables[i->get_name()] = get_drawable_factory(i)->create_drawable();

        drawables[i->get_name()]->draw(
            projection_matrix, view_matrix, i->model_matrix(), light_pos, cam_pos);
    }

    FrameBuffer::unbind();
}

void OpenGlWindow::draw_imgui_image() {
    if (ImGui::BeginTabItem(name.c_str(), &opened)) {

        const auto start_pos = ImGui::GetCursorStartPos();
        window_start_x = start_pos.x;
        window_start_y = start_pos.y;

        on_imgui_tab_begin();

        if (ImGui::IsWindowHovered()) camera->update();

        ImGui::Image(
            frame_buffer->get_frame_texture(), ImGui::GetContentRegionAvail(), ImVec2(0, 1),
            ImVec2(1, 0));

        active = true;

        ImGui::EndTabItem();
    } else {
        active = false;
    }
}

std::tuple<float, float> OpenGlWindow::get_window_start_pos() {
    return {window_start_x, window_start_y};
}

std::tuple<float, float> OpenGlWindow::get_width_height() { return {window_width, window_height}; }

bool OpenGlWindow::is_active() const { return active; }

bool OpenGlWindow::is_opened() const { return opened; }

glm::mat4 OpenGlWindow::get_view_matrix() { return view_matrix; }
glm::mat4 OpenGlWindow::get_projection_matrix() { return projection_matrix; }

std::shared_ptr<DrawableFactory>
OpenGlWindow::get_drawable_factory(const std::shared_ptr<ShapeItem> &item, std::mt19937 &curr_rng) {
    std::shared_ptr<DrawableFactory> factory;
    const auto shape = item->get_shape();

    switch (item->get_drawable_kind()) {
        case SPECULAR:
            factory = std::make_shared<ObjSpecularFactory>(
                shape->get_vertices(), shape->get_normals(), rng, 300.f);
            break;
        case TILE_SPECULAR:
            factory = std::make_shared<TileGroundFactory>(
                shape->get_vertices(), shape->get_normals(), rng, 300.f, 1.f);
            break;
    }
    return factory;
}

std::shared_ptr<DrawableFactory>
OpenGlWindow::get_drawable_factory(const std::shared_ptr<NoShapeItem> &item) {
    std::shared_ptr<DrawableFactory> factory;
    switch (item->get_drawable_kind()) {
        case BASIS_AXIS: factory = std::make_shared<BasisAxisFactory>(); break;
        case ROTATION_TORUS: factory = std::make_shared<RotationTorusFactory>(); break;
    }
    return factory;
}

std::shared_ptr<Environment> OpenGlWindow::get_env() { return env; }

/*
 * Builder
 */

BuilderOpenGlWindow::BuilderOpenGlWindow(
    const std::shared_ptr<ItemFocusContext> &context, std::string bar_item_name,
    const std::shared_ptr<RobotBuilderEnvironment> &env,
    const std::function<
        void(std::string, std::optional<std::string>, std::shared_ptr<RobotBuilderEnvironment>)>
        &on_member_focused,
    const std::function<
        void(std::string, std::optional<std::string>, std::shared_ptr<RobotBuilderEnvironment>)>
        &on_constraint_focused,
    const std::function<PartKind()> &get_part_type)
    : OpenGlWindow(std::move(bar_item_name), env), context(context),
      mouse_event(std::make_unique<RayMouseEvent>(1920, 1080)), builder_env(env),
      on_member_focused(on_member_focused), on_constraint_focused(on_constraint_focused),
      get_part_type(get_part_type) {}

void BuilderOpenGlWindow::on_imgui_tab_begin() {
    if (ImGui::IsWindowHovered() && ImGui::IsWindowFocused()) {
        const auto ray_coords = mouse_event->get_scene_absolute_click_pos(
            ImGui::GetCursorPosX(), ImGui::GetCursorPosX());

        if (ray_coords.has_value()) {
            const auto &[near, far] = ray_coords.value();

            switch (get_part_type()) {
                case MEMBER:
                    on_member_focused(
                        get_name(), builder_env->ray_cast_member(near, far), builder_env);
                    break;
                case CONSTRAINT:
                    on_constraint_focused(
                        get_name(), builder_env->ray_cast_constraint(near, far), builder_env);
                    break;
                case MUSCLE: break;
            };
        }
    }
}

void BuilderOpenGlWindow::on_opengl_frame(
    const float new_width, const float new_height, const glm::mat4 &new_view_matrix,
    const glm::mat4 &new_proj_matrix) {

    mouse_event->update(new_width, new_height, new_view_matrix, new_proj_matrix);
}

std::shared_ptr<DrawableFactory> BuilderOpenGlWindow::get_drawable_factory(
    const std::shared_ptr<ShapeItem> &item, std::mt19937 &curr_rng) {
    if (item->get_drawable_kind() == SPECULAR)
        return std::make_shared<BuilderObjSpecularFactory>(
            item->get_shape()->get_vertices(), item->get_shape()->get_normals(), rng, 300.f,
            [this, item]() { return context->get_focus_color(item->get_name()); },
            [this, item]() {
                switch (get_part_type()) {
                    case MEMBER: return !builder_env->member_exists(item->get_name());
                    case CONSTRAINT: return !builder_env->constraint_exists(item->get_name());
                    case MUSCLE: return false;
                }
                return false;
            });

    return OpenGlWindow::get_drawable_factory(item, curr_rng);
}

/*
 * Run
 */

InferOpenGlWindow::InferOpenGlWindow(
    const std::shared_ptr<Agent> &agent, std::string bar_item_name,
    const std::shared_ptr<Environment> &env)
    : OpenGlWindow(std::move(bar_item_name), env), agent(agent), curr_step(get_env()->reset()) {}

void InferOpenGlWindow::on_opengl_frame(
    float new_width, float new_height, const glm::mat4 &new_view_matrix,
    const glm::mat4 &new_proj_matrix) {

    if (curr_step.done) curr_step = get_env()->reset();

    const auto action = agent->act(curr_step.state, curr_step.reward);
    curr_step = get_env()->do_step(action);
}

void InferOpenGlWindow::on_imgui_tab_begin() {}
