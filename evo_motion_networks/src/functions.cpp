//
// Created by samuel on 07/04/24.
//

#include <cmath>

#include <evo_motion_networks/functions.h>

torch::Tensor rand_eps(const torch::Tensor &tensor_like, const float epsilon) {
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

torch::Tensor truncated_normal_log_pdf(
    const torch::Tensor &x, const torch::Tensor &mu, const torch::Tensor &sigma, float min_value,
    float max_value) {
    const auto alpha = (min_value - mu) / sigma;
    const auto beta = (max_value - mu) / sigma;

    const auto z = theta(beta) - theta(alpha);

    return -0.5f * std::log(2 * static_cast<float>(M_PI)) - torch::log(sigma)
           - 0.5 * torch::pow((x - mu) / sigma, 2.f) - torch::log(z);
}

torch::Tensor truncated_normal_cdf(
    const torch::Tensor &x, const torch::Tensor &mu, const torch::Tensor &sigma, float min_value,
    float max_value) {
    const auto xi = (x - mu) / sigma;
    const auto alpha = (min_value - mu) / sigma;
    const auto beta = (max_value - mu) / sigma;
    const auto z = theta(beta) - theta(alpha);

    return (theta(xi) - theta(alpha)) / z;
}

torch::Tensor truncated_normal_cdf_interval(
    const torch::Tensor &x, const torch::Tensor &mu, const torch::Tensor &sigma, float min_value,
    float max_value, float epsilon) {
    return truncated_normal_cdf(x + epsilon, mu, sigma, min_value, max_value)
           - truncated_normal_cdf(x - epsilon, mu, sigma, min_value, max_value);
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
    const torch::Tensor &mu, const torch::Tensor &sigma, const float min_value,
    const float max_value) {
    const auto alpha = (min_value - mu) / sigma;
    const auto beta = (max_value - mu) / sigma;

    const auto z = theta(beta) - theta(alpha);

    return torch::log(sqrt(2.0 * M_PI * std::exp(1.0)) * sigma * z)
           + 0.5 * (alpha * phi(alpha) - beta * phi(beta)) / z;
}

float exponential_decrease(const long t, const long max_t, const float start, const float end) {
    const auto k = -std::log(end / start) / static_cast<float>(max_t);
    return std::max(start * std::exp(-k * static_cast<float>(t)), end);
}

/*
 * Torch module functions
 */

void hard_update(
    const std::shared_ptr<torch::nn::Module> &to, const std::shared_ptr<torch::nn::Module> &from) {
    for (auto n_p: from->named_parameters()) {
        const auto &name = n_p.key();
        const auto &param = n_p.value();
        to->named_parameters()[name].data().copy_(param.data());
    }
}

void soft_update(
    const std::shared_ptr<torch::nn::Module> &to, const std::shared_ptr<torch::nn::Module> &from,
    const float tau) {
    for (auto n_p: from->named_parameters()) {
        const auto &name = n_p.key();
        const auto &param = n_p.value();

        to->named_parameters()[name].data().copy_(
            tau * param.data() + (1.f - tau) * to->named_parameters()[name].data());
    }
}
