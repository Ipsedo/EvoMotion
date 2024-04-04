//
// Created by samuel on 28/01/24.
//

#include <memory>
#include <random>
#include "./test_muscle.h"
#include "./view/camera.h"
#include "./view/renderer.h"
#include "./view/specular.h"
#include "./env/env_test_muscle.h"

void test_muscle(muscle_params params) {

    auto env = std::make_shared<MuscleEnv>();

    std::shared_ptr<Camera> camera = std::make_shared<StaticCamera>(
        glm::vec3(1.f, 1.f, -1.f),
        glm::normalize(glm::vec3(1.f, 0.f, 1.f)),
        glm::vec3(0.f, 1.f, 0.f)
    );

    Renderer renderer(
        "evo_motion",
        params.width,
        params.height,
        camera
    );

    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_real_distribution<float> dist(0.f, 1.f);

    for (auto i: env->get_items()) {

        std::shared_ptr<OBjSpecular> specular = std::make_shared<OBjSpecular>(
            i.get_shape()->get_vertices(),
            i.get_shape()->get_normals(),
            glm::vec4(dist(rng), dist(rng), dist(rng), .5f),
            glm::vec4(dist(rng), dist(rng), dist(rng), .5f),
            glm::vec4(dist(rng), dist(rng), dist(rng), .5f),
            300.f
        );

        renderer.add_drawable(i.get_name(), specular);
    }

    step step = env->reset();
    while (!renderer.is_close()) {
        step = env->do_step(-torch::ones({1}), 1.f / 1000.f);

        std::map<std::string, glm::mat4> model_matrix;

        for (auto i: env->get_items())
            model_matrix.insert({i.get_name(), i.model_matrix()});

        renderer.draw(model_matrix);

        if (step.done) {
            // step = env->reset();
            std::cout << "reset" << std::endl;
        }
    }

}
