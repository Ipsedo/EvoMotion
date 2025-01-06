//
// Created by samuel on 11/11/24.
//

#ifndef EVO_MOTION_ENTROPY_H
#define EVO_MOTION_ENTROPY_H

#include <torch/torch.h>

class EntropyParameter final : public torch::nn::Module {
public:
    EntropyParameter(
        float initial_alpha, int nb_parameters, float min_alpha = 1e-8, float max_alpha = 1.f);
    torch::Tensor log_alpha();
    torch::Tensor alpha();

private:
    torch::Tensor log_alpha_t;
    float min_log;
    float max_log;
};

#endif//EVO_MOTION_ENTROPY_H
