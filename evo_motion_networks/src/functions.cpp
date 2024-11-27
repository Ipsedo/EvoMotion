//
// Created by samuel on 07/04/24.
//

#include <cmath>

#include <evo_motion_networks/functions.h>

torch::Tensor rand_eps(const torch::Tensor &tensor_like, const float epsilon) {
    return epsilon
           + torch::rand_like(tensor_like, at::TensorOptions(tensor_like.device()))
                 * (1.0 - 2.0 * epsilon);
}

torch::Tensor
normal_pdf(const torch::Tensor &x, const torch::Tensor &mu, const torch::Tensor &sigma) {
    return torch::exp(-0.5 * torch::pow((x - mu) / sigma, 2.0)) / (sigma * sqrt(2.0 * M_PI));
}

torch::Tensor
normal_cdf(const torch::Tensor &x, const torch::Tensor &mu, const torch::Tensor &sigma) {
    return 0.5 * (1.0 + torch::erf((x - mu) / (sigma * std::sqrt(2.0))));
}

torch::Tensor phi(const torch::Tensor &z) {
    return torch::exp(-0.5 * torch::pow(z, 2.0)) / std::sqrt(2.0 * M_PI);
}

torch::Tensor theta(const torch::Tensor &x) { return 0.5 * (1.0 + torch::erf(x / std::sqrt(2.0))); }

torch::Tensor theta_inv(const torch::Tensor &theta) {
    return std::sqrt(2.0) * torch::erfinv(2.0 * theta - 1.0);
}

torch::Tensor truncated_normal_pdf(
    const torch::Tensor &x, const torch::Tensor &mu, const torch::Tensor &sigma,
    const float min_value, const float max_value) {
    const auto alpha = (min_value - mu) / sigma;
    const auto beta = (max_value - mu) / sigma;
    return phi((x - mu) / sigma) / ((theta(beta) - theta(alpha)) * sigma);
}

torch::Tensor truncated_normal_log_pdf(
    const torch::Tensor &x, const torch::Tensor &mu, const torch::Tensor &sigma,
    const float min_value, const float max_value) {
    const auto alpha = (min_value - mu) / sigma;
    const auto beta = (max_value - mu) / sigma;

    const auto z = theta(beta) - theta(alpha);

    return -0.5 * std::log(2 * static_cast<float>(M_PI)) - torch::log(sigma)
           - 0.5 * torch::pow((x - mu) / sigma, 2.0) - torch::log(z);
}

torch::Tensor truncated_normal_cdf(
    const torch::Tensor &x, const torch::Tensor &mu, const torch::Tensor &sigma,
    const float min_value, const float max_value) {
    const auto xi = (x - mu) / sigma;
    const auto alpha = (min_value - mu) / sigma;
    const auto beta = (max_value - mu) / sigma;
    const auto z = theta(beta) - theta(alpha);

    return (theta(xi) - theta(alpha)) / z;
}

torch::Tensor truncated_normal_cdf_interval(
    const torch::Tensor &x, const torch::Tensor &mu, const torch::Tensor &sigma,
    const float min_value, const float max_value, const float epsilon) {
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

/*
 * Entropy factor
 */

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
            tau * param.data() + (1.0 - tau) * to->named_parameters()[name].data());
    }
}

/*
 * Count parameters
 */

int count_module_parameters(const std::shared_ptr<torch::nn::Module> &module) {
    const auto params = module->parameters();
    return std::accumulate(
        params.begin(), params.end(), 0, [](const int acc, const torch::Tensor &p) {
            const auto sizes = p.sizes();
            return acc + std::reduce(sizes.begin(), sizes.end(), 1, std::multiplies<>());
        });
}
