//
// Created by samuel on 17/12/22.
//
#include <iostream>
#include <utility>

#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>

#include <evo_motion_view/renderer.h>

void GLAPIENTRY message_callback(
    const GLenum source, const GLenum type, const GLuint id, const GLenum severity,
    const GLsizei length, const GLchar *message, const void *userParam) {
    std::cerr << source << " " << type << " " << id << " " << severity << " " << length << " : "
              << std::endl;
    std::cerr << "params : " << userParam << std::endl;
    std::cerr << message << std::endl << std::endl;
}

Renderer::Renderer(
    const std::string &title, const int width, const int height, std::shared_ptr<Camera> camera)
    : title(title), width(width), height(height), is_open(false),
      light_pos(glm::vec3(0.f, 0.f, -1.f)), camera(std::move(camera)), drawables({}),
      window(nullptr), vao(0) {

    if (!glfwInit()) {
        std::cerr << "GLFW initialization failed" << std::endl;
        exit(1);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
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
    glEnable(GL_MULTISAMPLE);

    glDepthFunc(GL_LEQUAL);

    glDepthMask(GL_TRUE);

    glGenVertexArrays(1, &vao);

    glDebugMessageCallback(message_callback, nullptr);

    is_open = true;
}

Renderer::~Renderer() {
    glfwDestroyWindow(window);
    glfwTerminate();
}

void Renderer::add_drawable(const std::string &name, const std::shared_ptr<Drawable> &drawable) {
    drawables.insert({name, drawable});
}

bool Renderer::is_close() const { return !is_open; }

void Renderer::close() {
    glfwSetWindowShouldClose(window, true);
    is_open = false;
}

void Renderer::draw(const std::map<std::string, glm::mat4> &model_matrix, const float delta_t) {
    if (glfwWindowShouldClose(window)) {
        is_open = false;
        return;
    }

    glfwPollEvents();

    on_new_frame();

    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindVertexArray(vao);
    render_drawables(model_matrix, delta_t);

    on_end_frame();

    glfwSwapBuffers(window);
}

void Renderer::render_drawables(
    std::map<std::string, glm::mat4> model_matrix, const float delta_t) {
    camera->step(delta_t);

    const glm::mat4 view_matrix = glm::lookAt(camera->pos(), camera->look(), camera->up());

    const glm::mat4 proj_matrix = glm::frustum(
        -1.f, 1.f, -static_cast<float>(height) / static_cast<float>(width),
        static_cast<float>(height) / static_cast<float>(width), 1.f, 200.f);

    for (const auto &[name, drawable]: drawables)
        drawable->draw(proj_matrix, view_matrix, model_matrix[name], light_pos, camera->pos());
}

void Renderer::on_new_frame() {}
void Renderer::on_end_frame() {}

void Renderer::reset_camera() const { camera->reset(); }
