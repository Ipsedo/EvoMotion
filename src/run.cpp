//
// Created by samuel on 20/12/22.
//

#include <random>

#include "./run.h"
#include "./networks/actor_critic.h"
#include "./env/cartpole.h"
#include "./view/renderer.h"
#include "./view/specular.h"

void infer(int seed, bool cuda, const run_params &params) {
    CartPole cart_pole(seed);

    std::shared_ptr<Camera> camera = std::make_shared<StaticCamera>(
            glm::vec3(0.f, 0.f, -1.f),
            glm::vec3(0.f, 0.f, 1.f),
            glm::vec3(0.f, 1.f, 0.f)
    );

    Renderer renderer(
            "evo_motion",
            params.window_width,
            params.window_height,
            camera
    );

    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_real_distribution<float> dist(0.f, 1.f);

    for (auto i: cart_pole.get_items()) {

        std::shared_ptr<OBjSpecular> specular = std::make_shared<OBjSpecular>(
                i.get_shape()->get_vertices(),
                i.get_shape()->get_normals(),
                glm::vec4(dist(rng), dist(rng), dist(rng), 1.f),
                glm::vec4(dist(rng), dist(rng), dist(rng), 1.f),
                glm::vec4(dist(rng), dist(rng), dist(rng), 1.f),
                300.f
        );

        renderer.add_drawable(i.get_name(), specular);
    }

    ActorCritic a2c(0,
                    cart_pole.get_state_space(),
                    cart_pole.get_action_space(),
                    32,
                    1e-4f
    );

    a2c.load(params.input_folder);

    if (cuda) {
        a2c.to(torch::kCUDA);
        cart_pole.to(torch::kCUDA);
    }

    step step = cart_pole.reset();
    while (!renderer.is_close()) {
        step = cart_pole.do_step(a2c.act(step), 1.f / 60.f);

        std::map<std::string, glm::mat4> model_matrix;

        for (auto i: cart_pole.get_items())
            model_matrix.insert({i.get_name(), i.model_matrix()});

        renderer.draw(model_matrix);

        if (step.done) {
            step = cart_pole.reset();
            std::cout << "reset" << std::endl;
        }
    }
}
