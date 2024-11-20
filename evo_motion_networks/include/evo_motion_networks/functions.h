//
// Created by samuel on 07/04/24.
//

#ifndef EVO_MOTION_FUNCTIONS_H
#define EVO_MOTION_FUNCTIONS_H

#include <torch/torch.h>

torch::Tensor rand_eps(const torch::Tensor &tensor_like, float epsilon = 1e-8f);

torch::Tensor
normal_pdf(const torch::Tensor &x, const torch::Tensor &mu, const torch::Tensor &sigma);

torch::Tensor
normal_cdf(const torch::Tensor &x, const torch::Tensor &mu, const torch::Tensor &sigma);

/*
 * truncated normal
 */

torch::Tensor truncated_normal_sample(
    const torch::Tensor &mu, const torch::Tensor &sigma, float min_value, float max_value);

torch::Tensor truncated_normal_pdf(
    const torch::Tensor &x, const torch::Tensor &mu, const torch::Tensor &sigma, float min_value,
    float max_value);

torch::Tensor truncated_normal_log_pdf(
    const torch::Tensor &x, const torch::Tensor &mu, const torch::Tensor &sigma, float min_value,
    float max_value);

torch::Tensor truncated_normal_cdf(
    const torch::Tensor &x, const torch::Tensor &mu, const torch::Tensor &sigma, float min_value,
    float max_value);

torch::Tensor truncated_normal_cdf_interval(
    const torch::Tensor &x, const torch::Tensor &mu, const torch::Tensor &sigma, float min_value,
    float max_value, float epsilon);

torch::Tensor truncated_normal_entropy(
    const torch::Tensor &mu, const torch::Tensor &sigma, float min_value, float max_value);

/*
 * exponential decrease
 */

float exponential_decrease(long t, long max_t, float start, float end);

/*
 * Torch module function
 */

void hard_update(
    const std::shared_ptr<torch::nn::Module> &to, const std::shared_ptr<torch::nn::Module> &from);
void soft_update(
    const std::shared_ptr<torch::nn::Module> &to, const std::shared_ptr<torch::nn::Module> &from,
    float tau);

#endif//EVO_MOTION_FUNCTIONS_H