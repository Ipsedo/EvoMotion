//
// Created by samuel on 20/12/22.
//

#include <filesystem>

#include <indicators/progress_bar.hpp>

#include <evo_motion_model/env_builder.h>
#include <evo_motion_networks/actor_critic_liquid.h>
#include <evo_motion_networks/metrics.h>

#include "./train.h"

void train(int seed, bool cuda, const train_params &params) {

    if (!std::filesystem::exists(params.output_path))
        std::filesystem::create_directory(params.output_path);
    else if (!std::filesystem::is_directory(params.output_path)) {
        std::cerr << "'" << params.output_path << "' is not a directory" << std::endl;
        exit(1);
    }

    EnvBuilder env_builder(seed, params.env_name);
    std::shared_ptr<Environment> env = env_builder.get();

    ActorCriticLiquid a2c(
        seed, env->get_state_space(), env->get_action_space(), params.hidden_size,
        params.learning_rate);

    if (cuda) {
        a2c.to(torch::kCUDA);
        env->to(torch::kCUDA);
    }

    a2c.set_eval(false);

    step step = env->reset();

    LossMeter actor_loss_meter("actor_loss", 32);
    LossMeter critic_loss_meter("critic_loss", 32);

    for (int s = 0; s < params.nb_saves; s++) {

        indicators::ProgressBar p_bar{
            indicators::option::MinProgress{0},
            indicators::option::MaxProgress{params.nb_episodes},
            indicators::option::BarWidth{100},
            indicators::option::Start{"["},
            indicators::option::Fill{"="},
            indicators::option::Lead{">"},
            indicators::option::Remainder{" "},
            indicators::option::End{"]"},
            indicators::option::ShowPercentage{true},
            indicators::option::ShowElapsedTime{true},
            indicators::option::ShowRemainingTime{true}};

        for (int e = 0; e < params.nb_episodes; e++) {

            while (!step.done) step = env->do_step(a2c.act(step.state, step.reward));

            a2c.done(step.reward);
            step = env->reset();

            auto metrics = a2c.get_metrics();

            actor_loss_meter.add(metrics["actor_loss"]);
            critic_loss_meter.add(metrics["critic_loss"]);

            std::string p_bar_description =
                "Save " + std::to_string(s - 1) +
                ", actor = " + std::to_string(actor_loss_meter.loss()) +
                ", critic = " + std::to_string(critic_loss_meter.loss());

            p_bar.set_option(indicators::option::PostfixText{p_bar_description});

            p_bar.tick();
        }

        // save agent
        auto save_folder_path =
            std::filesystem::path(params.output_path) / ("save_" + std::to_string(s));
        std::filesystem::create_directory(save_folder_path);
        a2c.save(save_folder_path);
    }
}
