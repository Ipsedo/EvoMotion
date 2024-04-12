//
// Created by samuel on 07/04/24.
//

#ifndef EVO_MOTION_FUNCTIONS_H
#define EVO_MOTION_FUNCTIONS_H

#include <torch/torch.h>

torch::Tensor truncated_normal_pdf(const torch::Tensor &xi);
torch::Tensor truncated_normal_cdf(const torch::Tensor &x);

torch::Tensor truncated_normal(
    const torch::Tensor &mu, const torch::Tensor &sigma, float min_value, float max_value);

#endif//EVO_MOTION_FUNCTIONS_H
