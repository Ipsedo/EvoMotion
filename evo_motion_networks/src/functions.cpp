//
// Created by samuel on 07/04/24.
//

#include "./functions.h"

#include <cmath>

torch::Tensor rand_eps(const torch::Tensor &tensor_like, float epsilon) {
    return epsilon
           + torch::rand_like(tensor_like, at::TensorOptions(tensor_like.device()))
           * (1.f - 2.f * epsilon);
}

torch::Tensor
normal_pdf(const torch::Tensor &x, const torch::Tensor &mu, const torch::Tensor &sigma) {
    return torch::exp(-0.5f * torch::pow((x - mu) / sigma, 2.f)) / (sigma * sqrt(2.f * M_PI));
}

torch::Tensor
normal_cdf(const torch::Tensor &x, const torch::Tensor &mu, const torch::Tensor &sigma) {
    return 0.5f * (1.f + torch::erf((x - mu) / (sigma * std::sqrt(2.f))));
}

torch::Tensor phi(const torch::Tensor &z) {
    return torch::exp(-0.5f * torch::pow(z, 2.f)) / std::sqrt(2.f * M_PI);
}

torch::Tensor theta(const torch::Tensor &x) { return 0.5f * (1.f + torch::erf(x / std::sqrt(2))); }

torch::Tensor theta_inv(const torch::Tensor &theta) {
    return std::sqrt(2.f) * torch::erfinv(2.f * theta - 1.f);
}

torch::Tensor truncated_normal_pdf(
    const torch::Tensor &x, const torch::Tensor &mu, const torch::Tensor &sigma,
    const float min_value, const float max_value) {
    const auto alpha = (min_value - mu) / sigma;
    const auto beta = (max_value - mu) / sigma;
    return phi((x - mu) / sigma) / ((theta(beta) - theta(alpha)) * sigma);
}

torch::Tensor truncated_normal_sample(
    const torch::Tensor &mu, const torch::Tensor &sigma, const float min_value,
    const float max_value) {
    const auto alpha = (min_value - mu) / sigma;
    const auto beta = (max_value - mu) / sigma;
    return theta_inv(
               theta(alpha)
               + at::rand(mu.sizes(), at::TensorOptions(mu.device()))
               * (theta(beta) - theta(alpha)))
           * sigma
           + mu;
}

torch::Tensor truncated_normal_entropy(
    const torch::Tensor &mu, const torch::Tensor &sigma, float min_value, float max_value) {
    const auto alpha = (min_value - mu) / sigma;
    const auto beta = (max_value - mu) / sigma;

    const auto z = theta(beta) - theta(alpha);

    return torch::log(sqrt(2.0 * M_PI * std::exp(1.0)) * sigma * z)
           + 0.5 * (alpha * phi(alpha) - beta * phi(beta)) / z;
}