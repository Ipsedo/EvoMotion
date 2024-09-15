//
// Created by samuel on 20/12/22.
//

#include <filesystem>
#include <iomanip>
#include <sstream>
#include <utility>

#include <indicators/progress_bar.hpp>

#include <evo_motion_model/env_builder.h>
#include <evo_motion_networks/agent_builder.h>
#include <evo_motion_networks/metrics.h>

#include "./run.h"

void train(int seed, bool cuda, const train_params &params) {

    if (!std::filesystem::exists(params.output_path))
        std::filesystem::create_directory(params.output_path);
    else if (!std::filesystem::is_directory(params.output_path)) {
        std::cerr << "'" << params.output_path << "' is not a directory" << std::endl;
        exit(1);
    }

    EnvBuilder env_builder(seed, params.env_name);
    std::shared_ptr<Environment> env = env_builder.get();

    AgentBuilder agent_builder(
        params.agent_name, seed, env->get_state_space(), env->get_action_space(),
        params.hidden_size, params.learning_rate);

    std::shared_ptr<Agent> agent = agent_builder.get();

    if (cuda) {
        agent->to(torch::kCUDA);
        env->to(torch::kCUDA);
    }

    agent->set_eval(false);

    step step = env->reset();

    std::cout << "state_space = " << env->get_state_space()
              << ", action_space = " << env->get_action_space() << std::endl;
    std::cout << "parameters_count = " << agent->count_parameters() << std::endl;

    LossMeter policy_loss_meter("policy_loss", 32);
    LossMeter policy_entropy_meter("policy_entropy", 32);
    LossMeter critic_loss_meter("critic_loss", 32);

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
            policy_entropy_meter.add(metrics["policy_entropy"]);
            critic_loss_meter.add(metrics["critic_loss"]);

            std::stringstream stream;
            stream << "Save " + std::to_string(s - 1)
                   << ", policy_loss = " << std::setprecision(6) << std::fixed << policy_loss_meter.loss()
                   << ", policy_entropy = " << std::setprecision(6) << std::fixed << policy_entropy_meter.loss()
                   << ", critic_loss = " << std::setprecision(6) << std::fixed << critic_loss_meter.loss()
                   << ", actor_grad_norm = " << std::setprecision(4) << std::fixed << metrics["actor_grad_mean"]
                   << ", critic_grad_norm = " << std::setprecision(4) << std::fixed << metrics["critic_grad_mean"]
                   << ", entropy_factor = " << std::setprecision(4) << std::fixed << metrics["entropy_factor"] << " ";

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