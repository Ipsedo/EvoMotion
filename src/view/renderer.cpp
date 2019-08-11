//
// Created by samuel on 11/08/19.
//

#include "renderer.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "error.h"


renderer::renderer(int width, int height) {
    width_px = width;
    height_px = height;
}

bool renderer::draw(float delta, std::vector<item> to_draw) {

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    for (item t : to_draw) {
        btScalar tmp[16];
        btTransform tr;
        t.m_rg_body->getMotionState()->getWorldTransform(tr);
        tr.getOpenGLMatrix(tmp);

        glm::mat4 model_mat = glm::make_mat4(tmp) * glm::scale(glm::mat4(1.f), t.m_obj_mtl_vbo_scale);

        glm::mat4 mv_mat = m_view_mat * model_mat;
        glm::mat4 mvp_mat = m_proj_mat * mv_mat;

        t.m_obj_mtl_vbo.draw(mvp_mat, mv_mat, glm::vec3(0.f), m_cam_pos);
    }

    glfwSwapBuffers(m_window);

    glfwPollEvents();

    return !glfwWindowShouldClose(m_window);
}


void renderer::init() {
    if (!glfwInit()) {
        fprintf(stderr, "Failed GLFW initialization\n");
        exit(0);
    }

    glfwDefaultWindowHints();
    glfwWindowHint(GLFW_SAMPLES, 4);
    m_window = glfwCreateWindow(int(width_px), int(height_px), "EvoMotion", NULL, NULL);
    if (!m_window) {
        fprintf(stderr, "Failed to open GLFW window.\n");
        glfwTerminate();
        exit(0);
    }
    glfwMakeContextCurrent(m_window);
    glfwSetErrorCallback(error_callback);
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        exit(0);
    }

    m_cam_pos = glm::vec3(0., 0., -1.);
    m_proj_mat = glm::frustum(-1.f, 1.f, -height_px / width_px, height_px / width_px, 1.0f, 200.0f);
    m_view_mat = glm::lookAt(
            glm::vec3(0., 0., 0.),
            glm::vec3(0., 0., 1.),
            glm::vec3(0, 1, 0)
    );

    glViewport(0, 0, int(width_px), int(height_px));
    glClearColor(0.5, 0.0, 0.0, 1.0);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_TRUE);
}
