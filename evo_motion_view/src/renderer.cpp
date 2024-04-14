//
// Created by samuel on 17/12/22.
//
#include <iostream>
#include <utility>

#include <glm/gtc/matrix_transform.hpp>

#include <evo_motion_view/renderer.h>

void GLAPIENTRY message_callback(
    GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message,
    const void *userParam) {
    std::cerr << source << " " << type << " " << id << " " << severity << " " << length << " : "
              << std::endl;
    std::cerr << "params : " << userParam << std::endl;
    std::cerr << message << std::endl << std::endl;
}

Renderer::Renderer(const std::string &title, int width, int height, std::shared_ptr<Camera> camera)
    : title(title), width(width), height(height), drawables({}), camera(std::move(camera)),
      is_open(false), window(nullptr), light_pos(glm::vec3(0.f, 0.f, -1.f)) {

    if (!glfwInit()) {
        std::cerr << "GLFW initialization failed" << std::endl;
        exit(1);
    }

    glfwDefaultWindowHints();
    glfwWindowHint(GLFW_REFRESH_RATE, 60);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);

    if (!window) {
        std::cerr << "GLFW window initialization failed" << std::endl;
        glfwTerminate();
        exit(1);
    }

    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK) {
        std::cerr << "GLEW initialization failed" << std::endl;
        glfwTerminate();
        exit(1);
    }

    glViewport(0, 0, width, height);

    glClearColor(0.5, 0., 0., 1.);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_MULTISAMPLE);

    glDepthFunc(GL_LEQUAL);

    glDepthMask(GL_TRUE);

    glDebugMessageCallback(message_callback, nullptr);

    is_open = true;
}

void Renderer::add_drawable(const std::string &name, const std::shared_ptr<Drawable> &drawable) {
    drawables.insert({name, drawable});
}

bool Renderer::is_close() const { return !is_open; }

void Renderer::close() {
    glfwSetWindowShouldClose(window, true);
    is_open = false;
}

void Renderer::draw(std::map<std::string, glm::mat4> model_matrix) {
    if (glfwWindowShouldClose(window)) {
        is_open = false;
        return;
    }

    glfwPollEvents();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 view_matrix = glm::lookAt(camera->pos(), camera->look(), camera->up());

    glm::mat4 proj_matrix = glm::frustum(
        -1.f, 1.f, -float(height) / float(width), float(height) / float(width), 1.f, 200.f);

    for (const auto &[name, drawable]: drawables) {
        glm::mat4 m_matrix = model_matrix[name];

        auto mv_matrix = view_matrix * m_matrix;
        auto mvp_matrix = proj_matrix * mv_matrix;

        drawable->draw(mvp_matrix, mv_matrix, light_pos, camera->pos());
    }

    glfwSwapBuffers(window);
}
