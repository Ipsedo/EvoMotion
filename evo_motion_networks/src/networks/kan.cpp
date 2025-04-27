//
// Created by samuel on 21/04/25.
//

#include <evo_motion_networks/networks/kan.h>

// Polynomial coefficients

PolyCoefActivation::PolyCoefActivation(const int n, const torch::Tensor &coefficients)
    : n(n), coefficients(register_buffer("coefficients", coefficients)),
      exponent(register_buffer("exponent", torch::arange(0, n + 1))) {}

int PolyCoefActivation::get_size() { return n; }

torch::Tensor PolyCoefActivation::forward(const torch::Tensor &x) {
    return torch::einsum("b...a,ac->bc...", {torch::pow(x.unsqueeze(-1), exponent), coefficients});
    ;
}

// Hermite

HermiteActivation::HermiteActivation(const int n) : PolyCoefActivation(n, hermite_coef(n)) {}

float HermiteActivation::factorial(const int n) {
    int result = 1;
    for (int i = n; i > 0; i--) { result *= i; }
    return static_cast<float>(result);
}

torch::Tensor HermiteActivation::hermite_coef(int n) {
    auto coef = torch::zeros({n + 1, n});
    for (int i = 0; i < n; i++) {
        const float i_float = static_cast<float>(i);
        for (int k = 0; k <= static_cast<int>(std::floor((i_float + 1.0) / 2.0)) + 1; k++) {
            const float k_float = static_cast<float>(k);
            coef[i + 1 - 2 * k][i] =
                std::pow(-1.0, k_float) / std::pow(2.0, k_float) / factorial(k)
                / factorial(i + 1 - 2 * k)
                * torch::exp(torch::lgamma(torch::tensor(i_float + 2.0)) / 2.0);
        }
    }
    return coef;
}

torch::Tensor HermiteActivation::forward(const torch::Tensor &x) {
    return torch::exp(-torch::pow(PolyCoefActivation::forward(x), 2.0) / 2.0);
}

// Linear KAN

LinearKAN::LinearKAN(
    int in_features, int out_features, const std::shared_ptr<ActivationFunction> &activation,
    const std::function<torch::Tensor(torch::Tensor)> &res_act_fun)
    : w_b(register_parameter("w_b", torch::ones({out_features, in_features}))),
      w_s(register_parameter("w_c", torch::ones({out_features, in_features}))),
      c(register_parameter("c", torch::ones({activation->get_size(), out_features, in_features}))),
      act(register_module("act", activation)), res_act_fun(res_act_fun) {

    torch::nn::init::xavier_normal_(w_b, 1e-1f);
    torch::nn::init::normal_(c.data(), 1e-1f);
}

torch::Tensor LinearKAN::forward(const torch::Tensor &x) {
    return torch::sum(
        torch::einsum("boi,oi->boi", {torch::einsum("bai,aoi->boi", {act->forward(x), c}), w_s})
            + torch::einsum("bi,oi->boi", {res_act_fun(x), w_b}),
        2);
}
