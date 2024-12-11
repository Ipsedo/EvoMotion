//
// Created by samuel on 20/12/22.
//

#include <chrono>
#include <random>
#include <thread>

#include <glm/glm.hpp>

#include <evo_motion_model/environment.h>
#include <evo_motion_networks/agent.h>
#include <evo_motion_view/camera.h>
#include <evo_motion_view/factory.h>
#include <evo_motion_view/renderer.h>

#include "./run.h"

void infer(
    int env_num_threads, int seed, bool cuda, const run_params &params,
    const std::shared_ptr<AgentFactory> &agent_factory,
    const std::shared_ptr<EnvironmentFactory> &environment_factory) {
    std::shared_ptr<Environment> env = environment_factory->get_env(env_num_threads, seed);

    std::shared_ptr<Camera> camera;
    if (env->get_camera_track_item().has_value()) {
        const auto track_item = env->get_camera_track_item().value();
        camera = std::make_shared<FollowCamera>([&track_item]() {
            return glm::vec3(track_item.model_matrix() * glm::vec4(glm::vec3(0), 1.f));
        });
    } else {
        camera = std::make_shared<StaticCamera>(
            glm::vec3(1.f, 1.f, -1.f), glm::normalize(glm::vec3(1.f, 0.f, 1.f)),
            glm::vec3(0.f, 1.f, 0.f));
    }

    Renderer renderer("evo_motion", params.window_width, params.window_height, camera);

    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_real_distribution dist(0.f, 1.f);

    for (const auto &i: env->get_items()) {
        std::shared_ptr<DrawableFactory> factory;

        switch (i.get_drawable_kind()) {
            case SPECULAR:
                factory = std::make_shared<ObjSpecularFactory>(
                    i.get_shape()->get_vertices(), i.get_shape()->get_normals(),
                    glm::vec4(dist(rng), dist(rng), dist(rng), 1.f),
                    glm::vec4(dist(rng), dist(rng), dist(rng), 1.f),
                    glm::vec4(dist(rng), dist(rng), dist(rng), 1.f), 300.f);
                break;
            case TILE_SPECULAR:
                factory = std::make_shared<TileGroundFactory>(
                    i.get_shape()->get_vertices(), i.get_shape()->get_normals(),
                    glm::vec4(dist(rng), dist(rng), dist(rng), 1.f),
                    glm::vec4(dist(rng), dist(rng), dist(rng), 1.f),
                    glm::vec4(dist(rng), dist(rng), dist(rng), 1.f),
                    glm::vec4(dist(rng), dist(rng), dist(rng), 1.f),
                    glm::vec4(dist(rng), dist(rng), dist(rng), 1.f),
                    glm::vec4(dist(rng), dist(rng), dist(rng), 1.f), 300.f, 1.f);
                break;
        }

        renderer.add_drawable(i.get_name(), factory->get_drawable());
    }

    std::shared_ptr<Agent> agent =
        agent_factory->create_agent(env->get_state_space(), env->get_action_space());

    agent->load(params.input_folder);

    if (cuda) {
        agent->to(torch::kCUDA);
        env->to(torch::kCUDA);
    }

    agent->set_eval(true);

    step step = env->reset();
    while (!renderer.is_close()) {
        auto before = std::chrono::system_clock::now();

        step = env->do_step(agent->act(step.state, step.reward));

        std::map<std::string, glm::mat4> model_matrix;

        for (const auto &i: env->get_items()) model_matrix.insert({i.get_name(), i.model_matrix()});

        renderer.draw(model_matrix, 1.f / 60.f);

        std::chrono::duration<double, std::milli> delta = std::chrono::system_clock::now() - before;

        std::this_thread::sleep_for(std::chrono::milliseconds(
            static_cast<long>(std::max(0., 1000. / 60. - delta.count()))));

        if (step.done) {
            agent->done(step.state, step.reward);
            step = env->reset();
            renderer.reset_camera();
            std::cout << "reset" << std::endl;
        }
    }
}