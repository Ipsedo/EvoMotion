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
      drawables(), env(env), active(true), opened(true),
      camera(std::make_unique<ImGuiCamera>([env]() {
          if (const auto track_item = env->get_camera_track_item(); track_item.has_value()) {
              return glm::vec3(track_item.value()->model_matrix_without_scale()[3]);
          }
          return glm::vec3(0.f);
      })) {}

std::string OpenGlWindow::get_name() { return name; }

void OpenGlWindow::rename_drawable(const std::string &old_name, const std::string &new_name) {
    auto n = drawables.extract(old_name);
    n.key() = new_name;
    drawables.insert(std::move(n));
}

void OpenGlWindow::draw_opengl(const float width, const float height) {
    const auto view_matrix = glm::lookAt(camera->pos(), camera->look(), camera->up());
    const auto projection_matrix =
        glm::frustum(-1.f, 1.f, -height / width, height / width, 1.f, 200.f);

    on_opengl_frame(width, height, view_matrix, projection_matrix);

    frame_buffer->rescale_frame_buffer(width, height);
    frame_buffer->bind();

    glViewport(0, 0, static_cast<int>(width), static_cast<int>(height));
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (const auto &i: env->get_draw_items()) {
        if (!drawables.contains(i->get_name()))
            drawables[i->get_name()] = get_drawable_factory(i, rng)->create_drawable();

        drawables[i->get_name()]->draw(
            projection_matrix, view_matrix, i->model_matrix(), glm::vec3(0, 20, 0), camera->pos());
    }

    FrameBuffer::unbind();
}

void OpenGlWindow::draw_imgui_image() {
    if (ImGui::BeginTabItem(name.c_str(), &opened)) {

        if (!active) on_open_tab();

        on_imgui_tab_begin();

        if (ImGui::IsWindowHovered()) camera->update();

        ImGui::Image(
            frame_buffer->get_frame_texture(), ImGui::GetContentRegionAvail(), ImVec2(0, 1),
            ImVec2(1, 0));

        active = true;

        ImGui::EndTabItem();
    } else {
        if (active) on_hide_tab();
        active = false;
    }

    if (!opened) on_hide_tab();
}

bool OpenGlWindow::is_active() const { return active; }

bool OpenGlWindow::is_opened() const { return opened; }

std::shared_ptr<DrawableFactory> OpenGlWindow::get_drawable_factory(
    const std::shared_ptr<AbstractItem> &item, std::mt19937 &curr_rng) {
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

std::shared_ptr<Environment> OpenGlWindow::get_env() { return env; }

/*
 * Builder
 */

BuilderOpenGlWindow::BuilderOpenGlWindow(
    const std::shared_ptr<AppContext> &context, std::string bar_item_name,
    const std::shared_ptr<RobotBuilderEnvironment> &env)
    : OpenGlWindow(std::move(bar_item_name), env), context(context),
      mouse_event(std::make_unique<RayMouseEvent>(1920, 1080)), builder_env(env) {}

void BuilderOpenGlWindow::on_imgui_tab_begin() {
    if (ImGui::IsWindowHovered() && ImGui::IsWindowFocused()) {
        const auto ray_coords = mouse_event->get_scene_absolute_click_pos(
            ImGui::GetCursorPosX(), ImGui::GetCursorPosX());

        if (ray_coords.has_value()) {
            const auto &[near, far] = ray_coords.value();
            if (!context->are_members_hidden()) {
                if (const auto result = builder_env->ray_cast_member(near, far); result.has_value())
                    context->set_focus_member(result.value());
                else context->release_focus_member();
            } else if (!context->are_constraints_hidden()) {
                if (const auto result = builder_env->ray_cast_constraint(near, far);
                    result.has_value())
                    context->set_focus_constraint(result.value());
                else context->release_focus_constraint();
            }
        }
    }
}

void BuilderOpenGlWindow::on_opengl_frame(
    const float new_width, const float new_height, const glm::mat4 &new_view_matrix,
    const glm::mat4 &new_proj_matrix) {

    mouse_event->update(new_width, new_height, new_view_matrix, new_proj_matrix);
}

std::shared_ptr<DrawableFactory> BuilderOpenGlWindow::get_drawable_factory(
    const std::shared_ptr<AbstractItem> &item, std::mt19937 &curr_rng) {
    if (item->get_drawable_kind() == SPECULAR)
        return std::make_shared<BuilderObjSpecularFactory>(
            item->get_shape()->get_vertices(), item->get_shape()->get_normals(), rng, 300.f,
            [this, item]() {
                if (builder_env->member_exists(item->get_name()))
                    return context->is_member_focused()
                           && context->get_focused_member() == item->get_name();
                if (builder_env->constraint_exists(item->get_name()))
                    return context->is_constraint_focused()
                           && context->get_focused_constraint() == item->get_name();

                return false;
            },
            [this, item]() {
                if (context->are_members_hidden() && builder_env->member_exists(item->get_name()))
                    return true;
                if (context->are_constraints_hidden()
                    && builder_env->constraint_exists(item->get_name()))
                    return true;
                return false;
            });

    return OpenGlWindow::get_drawable_factory(item, curr_rng);
}

void BuilderOpenGlWindow::on_hide_tab() {
    if (context->is_builder_env_selected()
        && context->get_builder_env()->get_robot_name() == builder_env->get_robot_name()) {
        context->release_focus_member();
        context->release_focus_constraint();
        context->release_builder_env();
    }
}

void BuilderOpenGlWindow::on_open_tab() {
    if (!context->is_builder_env_selected()
        || context->get_builder_env()->get_robot_name() != builder_env->get_robot_name()) {
        context->release_focus_member();
        context->release_focus_constraint();
        context->set_builder_env(builder_env);
    }
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
void InferOpenGlWindow::on_hide_tab() {}
void InferOpenGlWindow::on_open_tab() {}
