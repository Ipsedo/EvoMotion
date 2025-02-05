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

OpenGlWindow::OpenGlWindow(
    std::string bar_item_name, const std::shared_ptr<Environment> &env,
    const std::optional<std::function<void(glm::vec3, glm::vec3)>> &on_left_click)
    : name(std::move(bar_item_name)), frame_buffer(std::make_unique<FrameBuffer>(1920, 1080)),
      drawables(), env(env), rng(1234), rd_uni(0.f, 1.f), active(true),
      camera(std::make_unique<ImGuiCamera>([env]() {
          const auto track_item = env->get_camera_track_item();
          if (track_item.has_value()) {
              const auto pos = track_item.value().get_body()->getCenterOfMassPosition();
              return glm::vec3(pos.x(), pos.y(), pos.z());
          }
          return glm::vec3(0.f);
      })),
      mouse_event(std::make_unique<MouseEvent>(1920, 1080)),
      on_left_click(
          on_left_click.has_value() ? on_left_click.value()
                                    : [](const glm::vec3 &a, const glm::vec3 &b) {}) {}

void OpenGlWindow::draw_opengl(float width, float height) {
    frame_buffer->rescale_frame_buffer(width, height);
    frame_buffer->bind();

    glViewport(0, 0, static_cast<int>(width), static_cast<int>(height));
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const auto view_matrix = glm::lookAt(camera->pos(), camera->look(), camera->up());

    const auto projection_matrix =
        glm::frustum(-1.f, 1.f, -height / width, height / width, 1.f, 200.f);

    mouse_event->update(width, height, view_matrix, projection_matrix);

    for (const auto &i: env->get_items()) {
        if (drawables.find(i.get_name()) == drawables.end()) {
            std::shared_ptr<DrawableFactory> factory;

            switch (i.get_drawable_kind()) {
                case SPECULAR:
                    factory = std::make_shared<EdgeObjSpecularFactory>(
                        i.get_shape()->get_vertices(), i.get_shape()->get_normals(),
                        glm::vec4(rd_uni(rng), rd_uni(rng), rd_uni(rng), 1.f),
                        glm::vec4(rd_uni(rng), rd_uni(rng), rd_uni(rng), 1.f),
                        glm::vec4(rd_uni(rng), rd_uni(rng), rd_uni(rng), 1.f), 300.f, [this, i]() {
                            return focus_item.has_value() && focus_item.value() == i.get_name();
                        });
                    break;
                case TILE_SPECULAR:
                    factory = std::make_shared<TileGroundFactory>(
                        i.get_shape()->get_vertices(), i.get_shape()->get_normals(),
                        glm::vec4(rd_uni(rng), rd_uni(rng), rd_uni(rng), 1.f),
                        glm::vec4(rd_uni(rng), rd_uni(rng), rd_uni(rng), 1.f),
                        glm::vec4(rd_uni(rng), rd_uni(rng), rd_uni(rng), 1.f),
                        glm::vec4(rd_uni(rng), rd_uni(rng), rd_uni(rng), 1.f),
                        glm::vec4(rd_uni(rng), rd_uni(rng), rd_uni(rng), 1.f),
                        glm::vec4(rd_uni(rng), rd_uni(rng), rd_uni(rng), 1.f), 300.f, 1.f);
                    break;
            }

            drawables[i.get_name()] = factory->get_drawable();
        }

        drawables[i.get_name()]->draw(
            projection_matrix, view_matrix, i.model_matrix(), glm::vec3(0, 20, 0), camera->pos());
    }

    frame_buffer->unbind();
}

bool OpenGlWindow::draw_imgui_image() {
    bool opened = true;

    if (ImGui::BeginTabItem(name.c_str(), &opened)) {
        if (ImGui::IsWindowHovered()) {
            camera->update();

            if (ImGui::IsWindowFocused()) {
                const auto ray_coords = mouse_event->get_scene_absolute_click_pos(
                    ImGui::GetCursorPosX(), ImGui::GetCursorPosX());

                if (ray_coords.has_value()) {
                    const auto &[near, far] = ray_coords.value();
                    on_left_click(near, far);
                }
            }
        }

        ImGui::Image(
            frame_buffer->get_frame_texture(), ImGui::GetContentRegionAvail(), ImVec2(0, 1),
            ImVec2(1, 0));

        active = true;

        ImGui::EndTabItem();
    } else {
        active = false;
    }

    return opened;
}

bool OpenGlWindow::is_active() const { return active; }

std::shared_ptr<Environment> OpenGlWindow::get_env() { return env; }

void OpenGlWindow::set_focus_item(const std::string &item_name) { focus_item = item_name; }
void OpenGlWindow::release_focus_item() { focus_item = std::nullopt; }
