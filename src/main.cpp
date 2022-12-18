//
// Created by samuel on 15/12/22.
//

#include <iostream>
#include <memory>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "./view/camera.h"
#include "./view/renderer.h"
#include "./view/specular.h"
#include "./env/cartpole.h"

int main(int argc, char **argv) {
    std::cout << "init" << std::endl;
    glewInit();
    glfwInit();
    std::cout << "window" << std::endl;

    std::shared_ptr<Camera> camera = std::make_shared<StaticCamera>(
            glm::vec3(0.f, 0.f, -1.f),
            glm::vec3(0.f, 0.f, 1.f),
            glm::vec3(0.f, 1.f, 0.f)
    );

    std::cout << "mtn" << std::endl;
    Renderer renderer("evo_motion", 1024, 1024, camera);
    std::cout << "mtn mtn" << std::endl;

    std::cout << "ici" << std::endl;

    CartPole cart_pole(1234);

    std::cout << "la" << std::endl;

    for (auto i : cart_pole.get_items()) {
        std::cout << i.get_name() << std::endl;
        std::shared_ptr<OBjSpecular> specular = std::make_shared<OBjSpecular>(
                i.get_shape()->get_vertices(),
                i.get_shape()->get_normals(),
                glm::vec4(0.f, 0.f, 1.f, 1.f),
                glm::vec4(0.f, 1.f, 0.f, 1.f),
                glm::vec4(1.f, 1.f, 1.f, 1.f),
                300.f
        );

        renderer.add_drawable(i.get_name(), specular);
    }

    std::cout << cart_pole.get_action_space() << std::endl;

    while (!renderer.is_close()) {
        std::map<std::string, glm::mat4> model_matrix;

        step s = cart_pole.do_step(torch::rand(cart_pole.get_action_space()), 1.f / 60.f);

        for (auto i : cart_pole.get_items())
            model_matrix.insert({i.get_name(), i.model_matrix()});

        renderer.draw(model_matrix);
    }

    return 0;
}