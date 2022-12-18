//
// Created by samuel on 15/12/22.
//

#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "./view/camera.h"
#include "./view/renderer.h"
#include "./view/specular.h"
#include "./model/shapes.h"

int main(int argc, char **argv) {

    ObjShape shape("/home/samuel/CLionProjects/EvoMotion/resources/obj/cube.obj");

    std::shared_ptr<Camera> camera = std::make_shared<StaticCamera>(
            glm::vec3(0.f, 0.f, -1.f),
            glm::vec3(0.f, 0.f, 1.f),
            glm::vec3(0.f, 1.f, 0.f)
            );
    Renderer renderer("evo_motion", 1024, 1024, camera);

    std::shared_ptr<OBjSpecular> specular = std::make_shared<OBjSpecular>(
            shape.get_vertices(),
            shape.get_normals(),
            glm::vec4(0.f, 0.f, 1.f, 1.f),
            glm::vec4(0.f, 1.f, 0.f, 1.f),
            glm::vec4(1.f, 1.f, 1.f, 1.f),
            300.f
            );

    renderer.add_drawable("cube", specular);

    float angle = 0;

    while (!renderer.is_close()) {
        glm::mat4 model_matrix =
                glm::translate(glm::mat4(1.f), glm::vec3(0.f, 0.f, 10.f)) *
                glm::rotate(glm::mat4(1.f), float(angle), glm::normalize(glm::vec3(0.5f, 0.5f, 0.5f)));

        renderer.draw(std::map<std::string, glm::mat4>({{"cube", model_matrix}}));

        angle += 0.01f;
    }

    return 0;
}