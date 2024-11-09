//
// Created by samuel on 20/12/22.
//

#ifndef EVO_MOTION_RUN_H
#define EVO_MOTION_RUN_H

#include <string>

#include <evo_motion_model/environment.h>
#include <evo_motion_networks/agent.h>

struct run_params {
    std::string input_folder;

    int window_width;
    int window_height;
};

struct train_params {
    std::string output_path;

    int nb_saves;
    int nb_episodes;
};

void infer(
    int seed, bool cuda, const run_params &params,
    const std::shared_ptr<AgentFactory> &agent_factory,
    const std::shared_ptr<EnvironmentFactory> &environment_factory);

void train(
    int seed, bool cuda, const train_params &params,
    const std::shared_ptr<AgentFactory> &agent_factory,
    const std::shared_ptr<EnvironmentFactory> &environment_factory);

#endif//EVO_MOTION_RUN_H