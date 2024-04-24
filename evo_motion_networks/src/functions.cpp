//
// Created by samuel on 07/04/24.
//

#include "./functions.h"

#include <cmath>

torch::Tensor
normal_pdf(const torch::Tensor &x, const torch::Tensor &mu, const torch::Tensor &sigma) {
    return torch::exp(-0.5f * torch::pow((x - mu) / sigma, 2.f)) / (sigma * sqrt(2.f * M_PI));
}

torch::Tensor
normal_cdf(const torch::Tensor &x, const torch::Tensor &mu, const torch::Tensor &sigma) {
    return 0.5f * (1.f + torch::erf((x - mu) / (sigma * std::sqrt(2.f))));
}

torch::Tensor phi(const torch::Tensor &z) {
    return torch::exp(-0.5f * torch::pow(z, 2.f)) / std::sqrt(2 * M_PI);
}

torch::Tensor theta(const torch::Tensor &x) { return 0.5f * (1.f + torch::erf(x / std::sqrt(2))); }

torch::Tensor theta_inv(const torch::Tensor &theta) {
    return std::sqrt(2.f) * torch::erfinv(2.f * theta - 1.f);
}

torch::Tensor truncated_normal_pdf(
    const torch::Tensor &x, const torch::Tensor &mu, const torch::Tensor &sigma, float min_value,
    float max_value) {
    auto alpha = (min_value - mu) / sigma;
    auto beta = (max_value - mu) / sigma;
    return (phi((x - mu) / sigma)) / (theta(beta) - theta(alpha)) / sigma;
}

torch::Tensor truncated_normal_sample(
    const torch::Tensor &mu, const torch::Tensor &sigma, float min_value, float max_value) {
    auto alpha = (min_value - mu) / sigma;
    auto beta = (max_value - mu) / sigma;
    return theta_inv(
               theta(alpha)
               + at::rand(mu.sizes(), at::TensorOptions(mu.device()))
               * (theta(beta) - theta(alpha)))
           + mu;
}
