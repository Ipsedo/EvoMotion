//
// Created by samuel on 28/01/24.
//

#include "./test_muscle.h"

#include <iostream>
#include <memory>
#include <random>

#include <glm/glm.hpp>

#include <evo_motion_model/env_builder.h>
#include <evo_motion_networks/agent_builder.h>
#include <evo_motion_view/camera.h>
#include <evo_motion_view/renderer.h>
#include <evo_motion_view/specular.h>

void test_muscle(muscle_params params) {

    auto env = EnvBuilder(1234, "test_muscle").get();

    std::shared_ptr<Camera> camera = std::make_shared<StaticCamera>(
        glm::vec3(1.f, 1.f, -3.f), glm::normalize(glm::vec3(1.f, 0.f, 1.f)),
        glm::vec3(0.f, 1.f, 0.f));

    Renderer renderer("evo_motion", params.width, params.height, camera);

    std::shared_ptr<Agent> random_agent =
        AgentBuilder("random", 1234, {1}, env->get_action_space(), 0, 0.f).get();

    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_real_distribution<float> dist(0.f, 1.f);

    for (auto i: env->get_items()) {

        std::shared_ptr<OBjSpecular> specular = std::make_shared<OBjSpecular>(
            i.get_shape()->get_vertices(), i.get_shape()->get_normals(),
            glm::vec4(dist(rng), dist(rng), dist(rng), .5f),
            glm::vec4(dist(rng), dist(rng), dist(rng), .5f),
            glm::vec4(dist(rng), dist(rng), dist(rng), .5f), 300.f);

        renderer.add_drawable(i.get_name(), specular);
    }

    step step = env->reset();

    while (!renderer.is_close()) {
        step = env->do_step(random_agent->act(torch::tensor({0}), 0.f));

        std::map<std::string, glm::mat4> model_matrix;

        for (auto i: env->get_items()) model_matrix.insert({i.get_name(), i.model_matrix()});

        renderer.draw(model_matrix);

        if (step.done) {
            step = env->reset();
            std::cout << "reset" << std::endl;
        }
    }
}
