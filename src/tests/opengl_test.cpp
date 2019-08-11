//
// Created by samuel on 11/08/19.
//
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <thread>
#include <chrono>

#include "../view/obj_mtl_vbo.h"
#include "../view/normal_map.h"

#include <GLFW/glfw3.h>

#include "opengl_test.h"
#include "../utils/res.h"
#include "../view/error.h"

void test_opengl() {
    std::cout << "OpenGL test" << std::endl;

    if (!glfwInit()) {
        fprintf(stderr, "Failed GLFW initialization\n");
        exit(0);
    }

    glfwDefaultWindowHints();
    glfwWindowHint(GLFW_SAMPLES, 4);
    GLFWwindow *window;
    window = glfwCreateWindow(1024, 768, "EvoMotion", NULL, NULL);
    if (!window) {
        fprintf(stderr, "Failed to open GLFW window.\n");
        glfwTerminate();
        exit(0);
    }
    glfwMakeContextCurrent(window);
    glfwSetErrorCallback(error_callback);
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        exit(0);
    }

    //ObjMtlVBO objVBO(get_res_folder() + EVOMOTION_SEP + "obj" + EVOMOTION_SEP + "whale.obj"); // Prendre Ipsedo/mediefight/ [...]
    /*ObjMtlVBO objMtlVBO(get_res_folder() + EVOMOTION_SEP + "obj" + EVOMOTION_SEP + "whale.obj",
                        get_res_folder() + EVOMOTION_SEP + "obj" + EVOMOTION_SEP + "whale.mtl",
                        true);*/
    NormalMapObj square_map(get_res_folder() + EVOMOTION_SEP + "obj" + EVOMOTION_SEP + "sphere.obj",
                             get_res_folder() + EVOMOTION_SEP + "tex" + EVOMOTION_SEP + "alien_color.png",
                             get_res_folder() + EVOMOTION_SEP + "tex" + EVOMOTION_SEP + "alien_norm.png");

    glm::mat4 projectionMatrix = glm::frustum(-1.f, 1.f, -768.f / 1024.f, 768.f / 1024.f, 1.0f, 50.0f);
    glm::mat4 viewMatrix = glm::lookAt(
            glm::vec3(0., 0., -1.),
            glm::vec3(0., 0., 1.),
            glm::vec3(0, 1, 0)
    );

    glViewport(0, 0, 1024, 768);
    glClearColor(0.5, 0.0, 0.0, 1.0);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_TRUE);

    float angle = 0.0f;
    while (!glfwWindowShouldClose(window)) {
        glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), angle += 1e-3, glm::vec3(0.5f, 0.5f, 0.f));

        glm::mat4 translate = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 1.f));

        glm::mat4 scale = glm::scale(glm::mat4(1.f), glm::vec3(0.5f));

        glm::vec3 cameraPosition = glm::vec3(0., 0., -1.);

        glm::mat4 modelMatrix = scale * translate * rotation;
        glm::mat4 mvMatrix = viewMatrix * modelMatrix;
        glm::mat4 mvpMatrix = projectionMatrix * mvMatrix;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //objVBO.draw(mvpMatrix, mvMatrix, glm::vec3(50.f*cos(angleCam),0.f,50.f*sin(angleCam)));
        //objMtlVBO.draw(mvpMatrix, mvMatrix, glm::vec3(0.f), cameraPosition);
        square_map.draw(mvpMatrix, mvMatrix, glm::vec3(0.f), cameraPosition);

        glfwSwapBuffers(window);

        glfwPollEvents();

        std::this_thread::sleep_for(std::chrono::milliseconds(1000 / 60));
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}