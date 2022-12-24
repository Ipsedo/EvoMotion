//
// Created by samuel on 20/12/22.
//

#ifndef EVO_MOTION_RUN_H
#define EVO_MOTION_RUN_H

#include <string>

struct run_params {
    std::string input_folder;

    int window_width;
    int window_height;
};

void infer(int seed, bool cuda, const run_params &params);

#endif //EVO_MOTION_RUN_H
