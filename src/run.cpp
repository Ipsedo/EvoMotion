//
// Created by samuel on 20/12/22.
//

#include <chrono>
#include <random>
#include <thread>

#include "./env/builder.h"
#include "./networks/actor_critic_liquid.h"
#include "./run.h"
#include "./view/renderer.h"
#include "./view/specular.h"

void infer(int seed, bool cuda, const run_params &params) {
    EnvBuilder env_builder(seed, params.env_name);
    std::shared_ptr<Environment> env = env_builder.get();

    std::shared_ptr<Camera> camera = std::make_shared<StaticCamera>(
        glm::vec3(1.f, 1.f, -1.f), glm::normalize(glm::vec3(1.f, 0.f, 1.f)),
        glm::vec3(0.f, 1.f, 0.f));

    Renderer renderer("evo_motion", params.window_width, params.window_height, camera);

    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_real_distribution<float> dist(0.f, 1.f);

    for (auto i: env->get_items()) {

        std::shared_ptr<OBjSpecular> specular = std::make_shared<OBjSpecular>(
            i.get_shape()->get_vertices(), i.get_shape()->get_normals(),
            glm::vec4(dist(rng), dist(rng), dist(rng), 1.f),
            glm::vec4(dist(rng), dist(rng), dist(rng), 1.f),
            glm::vec4(dist(rng), dist(rng), dist(rng), 1.f), 300.f);

        renderer.add_drawable(i.get_name(), specular);
    }

    ActorCriticLiquid a2c(
        0, env->get_state_space(), env->get_action_space(), params.hidden_size, 1e-4f);

    a2c.load(params.input_folder);

    if (cuda) {
        a2c.to(torch::kCUDA);
        env->to(torch::kCUDA);
    }

    a2c.set_eval(true);

    step step = env->reset();
    while (!renderer.is_close()) {
        auto before = std::chrono::system_clock::now();

        step = env->do_step(a2c.act(step));

        std::map<std::string, glm::mat4> model_matrix;

        for (auto i: env->get_items()) model_matrix.insert({i.get_name(), i.model_matrix()});

        renderer.draw(model_matrix);

        std::chrono::duration<double, std::milli> delta = std::chrono::system_clock::now() - before;

        std::this_thread::sleep_for(
            std::chrono::milliseconds(long(std::max(0., 1000. / 60. - delta.count()))));

        if (step.done) {
            step = env->reset();
            std::cout << "reset" << std::endl;
        }
    }
}
