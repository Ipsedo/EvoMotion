//
// Created by samuel on 20/12/22.
//

#ifndef EVO_MOTION_RUN_H
#define EVO_MOTION_RUN_H

#include <string>

struct run_params {
    std::string env_name;

    std::string input_folder;

    int window_width;
    int window_height;
};

struct train_params {
    std::string env_name;
    std::string output_path;

    int nb_saves;
    int nb_episodes;
};

void infer(
    int seed, bool cuda, const run_params &params,
    const std::shared_ptr<AgentFactory> &agent_factory);

void train(
    int seed, bool cuda, const train_params &params,
    const std::shared_ptr<AgentFactory> &agent_factory);

#endif//EVO_MOTION_RUN_H