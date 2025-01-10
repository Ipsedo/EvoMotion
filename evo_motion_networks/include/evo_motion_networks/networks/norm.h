//
// Created by samuel on 10/01/25.
//

#ifndef EVO_MOTION_NORM_H
#define EVO_MOTION_NORM_H

#include <torch/torch.h>

class BatchRenormalization final : public torch::nn::Module {
private:
    torch::Tensor running_mean;
    torch::Tensor running_std;

    torch::Tensor weight;
    torch::Tensor bias;

    float epsilon;
    float momentum;
    bool affine;

    long curr_step;
    long warmup_steps;

    float r_max_init;
    float r_max_end;
    float d_max_init;
    float d_max_end;

    float r_max();
    float d_max();

public:
    BatchRenormalization(
        int num_features, float epsilon = 1e-5, float momentum = 0.01, bool affine = true,
        int warmup_steps = 1e5);
    torch::Tensor forward(const torch::Tensor &x);
};

#endif//EVO_MOTION_NORM_H
