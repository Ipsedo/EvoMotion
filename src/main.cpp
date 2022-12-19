//
// Created by samuel on 15/12/22.
//

#include <iostream>
#include <memory>
#include <random>

#include "./networks/actor_critic.h"
#include "./view/camera.h"
#include "./view/renderer.h"
#include "./view/specular.h"
#include "./env/cartpole.h"

int main(int argc, char **argv) {

    std::shared_ptr<Camera> camera = std::make_shared<StaticCamera>(
            glm::vec3(0.f, 0.f, -1.f),
            glm::vec3(0.f, 0.f, 1.f),
            glm::vec3(0.f, 1.f, 0.f)
    );

    Renderer renderer("evo_motion", 1024, 1024, camera);

    CartPole cart_pole(1234);

    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_real_distribution<float> dist(0.f, 1.f);

    for (auto i : cart_pole.get_items()) {
        std::cout << i.get_name() << std::endl;
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

    ActorCritic a2c(
            cart_pole.get_state_space(),
            cart_pole.get_action_space(),
            32,
            1e-4f
            );

    step s = cart_pole.reset();

    while (!renderer.is_close()) {
        if (s.done) {
            a2c.done(s);
            s = cart_pole.reset();
        }
        std::map<std::string, glm::mat4> model_matrix;

        s = cart_pole.do_step(a2c.act(s), 1.f / 60.f);

        for (auto i : cart_pole.get_items())
            model_matrix.insert({i.get_name(), i.model_matrix()});

        renderer.draw(model_matrix);
    }

    return 0;
}