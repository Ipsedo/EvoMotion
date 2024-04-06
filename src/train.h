//
// Created by samuel on 20/12/22.
//

#ifndef EVO_MOTION_TRAIN_H
#define EVO_MOTION_TRAIN_H

#include <string>

struct train_params {
    std::string env_name;

    std::string output_path;

    float learning_rate;

    int nb_saves;
    int nb_episodes;

    int hidden_size;
};

void train(int seed, bool cuda, const train_params &params);

#endif//EVO_MOTION_TRAIN_H
