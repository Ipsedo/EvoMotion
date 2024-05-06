//
// Created by samuel on 17/12/22.
//

#ifndef EVO_MOTION_RENDERER_H
#define EVO_MOTION_RENDERER_H

#include <map>
#include <memory>
#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "./camera.h"
#include "./drawable.h"

class Renderer {
public:
    Renderer(const std::string &title, int width, int height, std::shared_ptr<Camera> camera);

    void add_drawable(const std::string &name, const std::shared_ptr<Drawable> &drawable);

    [[nodiscard]] bool is_close() const;

    void close();

    void draw(std::map<std::string, glm::mat4> model_matrix);

private:
    std::string title;

    int width;
    int height;

    bool is_open;

    glm::vec3 light_pos;
    std::shared_ptr<Camera> camera;

    std::map<std::string, std::shared_ptr<Drawable> > drawables;

    GLFWwindow *window;
};

#endif//EVO_MOTION_RENDERER_H
