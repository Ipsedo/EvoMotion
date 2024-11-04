//
// Created by samuel on 20/12/22.
//

#include <filesystem>
#include <iomanip>
#include <sstream>

#include <indicators/progress_bar.hpp>

#include <evo_motion_model/env_builder.h>
#include <evo_motion_networks/agent.h>
#include <evo_motion_networks/metrics.h>

#include "./run.h"

void train(
    int seed, bool cuda, const train_params &params,
    const std::shared_ptr<AgentFactory> &agent_factory) {

    if (!std::filesystem::exists(params.output_path))
        std::filesystem::create_directory(params.output_path);
    else if (!std::filesystem::is_directory(params.output_path)) {
        std::cerr << "'" << params.output_path << "' is not a directory" << std::endl;
        exit(1);
    }

    EnvBuilder env_builder(seed, params.env_name);
    std::shared_ptr<Environment> env = env_builder.get();

    std::shared_ptr<Agent> agent =
        agent_factory->create_agent(env->get_state_space(), env->get_action_space());

    if (cuda) {
        agent->to(torch::kCUDA);
        env->to(torch::kCUDA);
    }

    agent->set_eval(false);

    step step = env->reset();

    std::cout << "state_space = " << env->get_state_space()
              << ", action_space = " << env->get_action_space() << std::endl;
    std::cout << "parameters_count = " << agent->count_parameters() << std::endl;

    LossMeter policy_loss_meter("policy_loss", 128);
    LossMeter entropy_loss_meter("entropy_loss", 128);
    LossMeter critic_loss_meter("critic_loss", 128);
    LossMeter episode_steps_meter("episode_steps", 128);

    for (int s = 0; s < params.nb_saves; s++) {
        indicators::ProgressBar p_bar{
            indicators::option::MinProgress{0},
            indicators::option::MaxProgress{params.nb_episodes},
            indicators::option::BarWidth{30},
            indicators::option::Start{"["},
            indicators::option::Fill{"="},
            indicators::option::Lead{">"},
            indicators::option::Remainder{" "},
            indicators::option::End{"]"},
            indicators::option::ShowPercentage{true},
            indicators::option::ShowElapsedTime{true},
            indicators::option::ShowRemainingTime{true}};

        for (int e = 0; e < params.nb_episodes; e++) {
            while (!step.done) step = env->do_step(agent->act(step.state, step.reward));

            agent->done(step.reward);
            step = env->reset();

            auto metrics = agent->get_metrics();

            policy_loss_meter.add(metrics["policy_loss"]);
            entropy_loss_meter.add(metrics["entropy_loss"]);
            critic_loss_meter.add(metrics["critic_loss"]);
            episode_steps_meter.add(metrics["episode_steps"]);

            std::stringstream stream;
            stream << "Save " + std::to_string(s - 1) << ", policy = " << std::setprecision(6)
                   << std::fixed << policy_loss_meter.loss()
                   << ", entropy = " << std::setprecision(6) << std::fixed
                   << entropy_loss_meter.loss() << ", critic = " << std::setprecision(6)
                   << std::fixed << critic_loss_meter.loss()
                   << ", actor_grad_norm = " << std::setprecision(4) << std::fixed
                   << metrics["actor_grad_mean"] << ", critic_grad_norm = " << std::setprecision(4)
                   << std::fixed << metrics["critic_grad_mean"]
                   << ", steps = " << std::setprecision(2) << std::fixed
                   << episode_steps_meter.loss() << " ";

            p_bar.set_option(indicators::option::PrefixText{stream.str()});

            p_bar.tick();
        }

        // save agent
        auto save_folder_path =
            std::filesystem::path(params.output_path) / ("save_" + std::to_string(s));
        std::filesystem::create_directory(save_folder_path);
        agent->save(save_folder_path);
    }
}