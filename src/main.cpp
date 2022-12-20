//
// Created by samuel on 15/12/22.
//

#include <filesystem>
#include <iostream>
#include <memory>
#include <random>

#include <argparse/argparse.hpp>
#include <indicators/progress_bar.hpp>

#include "./networks/actor_critic.h"
#include "./view/camera.h"
#include "./view/renderer.h"
#include "./view/specular.h"
#include "./env/cartpole.h"

int main(int argc, char **argv) {
    argparse::ArgumentParser parser("evo_motion");

    parser.add_argument("output_path")
            .required()
            .help("output folder path (for models, metrics, etc)");

    parser.add_argument("-e", "--episodes")
        .scan<'i', int>()
        .default_value(1000)
        .help("episode number per save");

    parser.add_argument("-n", "--nb-save")
        .scan<'i', int>()
        .default_value(1000)
        .help("number of save when training");

    argparse::ArgumentParser view_parser("view");

    view_parser.add_argument("-w", "--width")
            .scan<'i', int>()
            .default_value(1024)
            .help("window width");

    view_parser.add_argument("-h", "--height")
            .scan<'i', int>()
            .default_value(1024)
            .help("window height");

    parser.add_subparser(view_parser);

    try {
        parser.parse_args(argc, argv);
    }
    catch (const std::runtime_error &err) {
        std::cerr << err.what() << std::endl;
        std::cerr << parser;
        std::exit(1);
    }

    std::string output_path = parser.get<std::string>("output_path");

    if (!std::filesystem::exists(output_path))
        std::filesystem::create_directory(output_path);
    else if (!std::filesystem::is_directory(output_path)) {
        std::cerr << "'" << output_path << "' is not a directory" << std::endl;
        exit(1);
    }

    CartPole cart_pole(1234);

    std::shared_ptr<Camera> camera = std::make_shared<StaticCamera>(
            glm::vec3(0.f, 0.f, -1.f),
            glm::vec3(0.f, 0.f, 1.f),
            glm::vec3(0.f, 1.f, 0.f)
    );

    std::unique_ptr<Renderer> renderer;
    if (parser.is_subcommand_used(view_parser)) {
        renderer = std::make_unique<Renderer>(
                "evo_motion",
                view_parser.get<int>("width"),
                view_parser.get<int>("height"),
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

            renderer->add_drawable(i.get_name(), specular);
        }
    }

    ActorCritic a2c(
            cart_pole.get_state_space(),
            cart_pole.get_action_space(),
            32,
            1e-4f
    );

    step step = cart_pole.reset();

    for (int s = 0; s < parser.get<int>("nb-save"); s++) {

        int nb_episode = parser.get<int>("episodes");
        int pb_bar_length = 100;
        int tick_every = nb_episode / pb_bar_length;
        indicators::ProgressBar p_bar{
            indicators::option::BarWidth(pb_bar_length),
            indicators::option::Start{"["},
            indicators::option::Fill{"="},
            indicators::option::Lead{">"},
            indicators::option::Remainder{" "},
            indicators::option::End{"]"},
        };

        for (int e = 0; e < nb_episode; e++) {

            while (!step.done) {
                step = cart_pole.do_step(a2c.act(step), 1.f / 60.f);

                if (parser.is_subcommand_used(view_parser) && !renderer->is_close()) {
                    std::map<std::string, glm::mat4> model_matrix;

                    for (auto i: cart_pole.get_items())
                        model_matrix.insert({i.get_name(), i.model_matrix()});

                    renderer->draw(model_matrix);
                }
            }

            a2c.done(step);
            step = cart_pole.reset();

            auto metrics = a2c.get_metrics();
            std::string p_bar_description =
                    "Save " + std::to_string(s)
                    + ", actor = " + std::to_string(metrics["actor_loss"])
                    + ", critic = " + std::to_string(metrics["critic_loss"]);
            p_bar.set_option(indicators::option::PostfixText{p_bar_description});

            if (e % tick_every == 0)
                p_bar.tick();
        }

        // save agent
        auto save_folder_path = std::filesystem::path(output_path) / ("save_" + std::to_string(s));
        std::filesystem::create_directory(save_folder_path);
        a2c.save(save_folder_path);
    }

    return 0;
}