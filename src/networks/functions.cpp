//
// Created by samuel on 07/04/24.
//

#include <cmath>

#include "./functions.h"


torch::Tensor truncated_normal_pdf(const torch::Tensor &xi) {
    return torch::exp(-xi / 2.f) / sqrt(2.f * M_PI);
}

torch::Tensor truncated_normal_cdf(const torch::Tensor &x) {
    return (1.f + torch::erf(x / std::sqrt(2.f))) / 2.f;
}


torch::Tensor truncated_normal(
    const torch::Tensor &mu, const torch::Tensor &sigma, float min_value, float max_value) {
    auto u = at::rand(mu.sizes(), at::TensorOptions(mu.device()));

    auto alpha = (min_value - mu) / sigma;
    auto beta = (max_value - mu) / sigma;
    auto phi_a = (1 + torch::erf(alpha / std::sqrt(2.f))) / 2.f;
    auto phi_b = (1 + torch::erf(beta / std::sqrt(2.f))) / 2.;

    return mu + sigma * torch::erfinv(2 * u * (phi_b - phi_a) + phi_a * 2.f - 1.f) * std::sqrt(2.f);
}
