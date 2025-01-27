//
// Created by samuel on 07/04/24.
//

#include <cmath>

#include <evo_motion_networks/functions.h>

#define SIGMA_MIN 1e-6f
#define SIGMA_MAX 1e6f
#define ALPHA_BETA_BOUND 5.f

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

    const auto safe_sigma = torch::clamp(sigma, SIGMA_MIN, SIGMA_MAX);

    const auto alpha =
        torch::clamp((min_value - mu) / safe_sigma, -ALPHA_BETA_BOUND, ALPHA_BETA_BOUND);
    const auto beta =
        torch::clamp((max_value - mu) / safe_sigma, -ALPHA_BETA_BOUND, ALPHA_BETA_BOUND);

    return phi((x - mu) / safe_sigma) / ((theta(beta) - theta(alpha)) * safe_sigma);
}

torch::Tensor truncated_normal_log_pdf(
    const torch::Tensor &x, const torch::Tensor &mu, const torch::Tensor &sigma,
    const float min_value, const float max_value) {

    const auto safe_sigma = torch::clamp(sigma, SIGMA_MIN, SIGMA_MAX);

    const auto alpha =
        torch::clamp((min_value - mu) / safe_sigma, -ALPHA_BETA_BOUND, ALPHA_BETA_BOUND);
    const auto beta =
        torch::clamp((max_value - mu) / safe_sigma, -ALPHA_BETA_BOUND, ALPHA_BETA_BOUND);

    const auto z = theta(beta) - theta(alpha);

    return -0.5 * std::log(2.0 * static_cast<double>(M_PI)) - torch::log(safe_sigma)
           - 0.5 * torch::pow((x - mu) / safe_sigma, 2.0) - torch::log(z);
}

torch::Tensor truncated_normal_cdf(
    const torch::Tensor &x, const torch::Tensor &mu, const torch::Tensor &sigma,
    const float min_value, const float max_value) {

    const auto safe_sigma = torch::clamp(sigma, SIGMA_MIN, SIGMA_MAX);

    const auto alpha =
        torch::clamp((min_value - mu) / safe_sigma, -ALPHA_BETA_BOUND, ALPHA_BETA_BOUND);
    const auto beta =
        torch::clamp((max_value - mu) / safe_sigma, -ALPHA_BETA_BOUND, ALPHA_BETA_BOUND);

    const auto xi = (x - mu) / safe_sigma;
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

    const auto safe_sigma = torch::clamp(sigma, SIGMA_MIN, SIGMA_MAX);

    const auto alpha =
        torch::clamp((min_value - mu) / safe_sigma, -ALPHA_BETA_BOUND, ALPHA_BETA_BOUND);
    const auto beta =
        torch::clamp((max_value - mu) / safe_sigma, -ALPHA_BETA_BOUND, ALPHA_BETA_BOUND);

    const auto cdf = torch::clamp(
        theta(alpha)
            + at::rand(mu.sizes(), at::TensorOptions(mu.device())) * (theta(beta) - theta(alpha)),
        0.f, 1.f);

    return torch::clamp(theta_inv(cdf) * safe_sigma + mu, min_value, max_value);
}

torch::Tensor truncated_normal_entropy(
    const torch::Tensor &mu, const torch::Tensor &sigma, const float min_value,
    const float max_value) {

    const auto safe_sigma = torch::clamp(sigma, SIGMA_MIN, SIGMA_MAX);

    const auto alpha =
        torch::clamp((min_value - mu) / safe_sigma, -ALPHA_BETA_BOUND, ALPHA_BETA_BOUND);
    const auto beta =
        torch::clamp((max_value - mu) / safe_sigma, -ALPHA_BETA_BOUND, ALPHA_BETA_BOUND);

    const auto z = theta(beta) - theta(alpha);

    return torch::log(sqrt(2.0 * M_PI * std::exp(1.0)) * safe_sigma * z)
           + 0.5 * (alpha * phi(alpha) - beta * phi(beta)) / z;
}

/*
 * KL divergence
 */

torch::Tensor kl_divergence(const torch::Tensor &log_p, const torch::Tensor &log_q) {
    return torch::exp(log_p) * (log_p - log_q);
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
 * Parameters functions
 */

int count_module_parameters(const std::shared_ptr<torch::nn::Module> &module) {
    const auto params = module->parameters();
    return std::accumulate(
        params.begin(), params.end(), 0, [](const int acc, const torch::Tensor &p) {
            const auto sizes = p.sizes();
            return acc + std::reduce(sizes.begin(), sizes.end(), 1, std::multiplies<>());
        });
}

float grad_norm_mean(const std::shared_ptr<torch::nn::Module> &module) {
    const auto params = module->parameters();
    return std::accumulate(
               params.begin(), params.end(), 0.f,
               [](const float acc, const torch::Tensor &p) {
                   return acc + p.grad().norm().item().toFloat();
               })
           / static_cast<float>(params.size());
}
