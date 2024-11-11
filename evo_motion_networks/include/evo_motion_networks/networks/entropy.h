//
// Created by samuel on 11/11/24.
//

#ifndef EVO_MOTION_ENTROPY_H
#define EVO_MOTION_ENTROPY_H

#include <torch/torch.h>

class EntropyParameter : public torch::nn::Module {
public:
    EntropyParameter();
    torch::Tensor log_alpha();
    torch::Tensor alpha();

private:
    torch::Tensor log_alpha_t;
};

#endif//EVO_MOTION_ENTROPY_H
