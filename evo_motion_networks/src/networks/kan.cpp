//
// Created by samuel on 21/04/25.
//

#include <evo_motion_networks/networks/kan.h>

// Polynomial coefficients

PolyCoefActivation::PolyCoefActivation(
    const int n, const torch::Tensor &coefficients, const float scale_coefficient)
    : n(n), coefficients(register_buffer("coefficients", coefficients)),
      exponent(register_buffer("exponent", torch::arange(0, n + 1))),
      scale_coefficients(register_buffer(
          "scale_coefficients",
          torch::pow(torch::tensor({scale_coefficient}), -torch::arange(0, n + 1)))) {}

int PolyCoefActivation::get_size() { return n; }

torch::Tensor PolyCoefActivation::forward(const torch::Tensor &x) {
    return torch::einsum(
        "b...a,ac->bc...",
        {torch::pow(x.unsqueeze(-1), exponent) * scale_coefficients, coefficients});
}

// Hermite

HermiteActivation::HermiteActivation(const int degree) : degree(degree) {}

/*float HermiteActivation::factorial(const int n) {
    int result = 1;
    for (int i = n; i > 0; i--) { result *= i; }
    return static_cast<float>(result);
}

torch::Tensor HermiteActivation::hermite_coef(int n) {
    auto coef = torch::zeros({n + 1, n});
    for (int i = 0; i < n; i++) {
        const auto i_float = static_cast<float>(i);
        for (int k = 0; k <= static_cast<int>(std::floor((i_float + 1.0) / 2.0)) + 1; k++) {
            const auto k_float = static_cast<float>(k);
            coef[i + 1 - 2 * k][i] =
                std::pow(-1.0, k_float) / std::pow(2.0, k_float) / factorial(k)
                / factorial(i + 1 - 2 * k)
                * torch::exp(torch::lgamma(torch::tensor(i_float + 2.0)) / 2.0);
        }
    }
    return coef;
}*/

int HermiteActivation::get_size() { return degree + 1; }

torch::Tensor HermiteActivation::forward(const torch::Tensor &x) {
    std::vector<torch::Tensor> hermites{
        torch::ones_like(x, torch::TensorOptions().dtype(torch::kFloat32).device(x.device()))};

    if (degree >= 1) hermites.push_back(2.0 * x / 10.);

    for (int n = 2; n <= degree; ++n) {
        torch::Tensor hn =
            2.0 * x * hermites[n - 1] - 2.0 * (static_cast<float>(n) - 1.0) * hermites[n - 2];
        hermites.push_back(hn / std::pow(10.0, n));
    }

    return torch::stack(hermites, 1);
}

// B-Spline

BSplinesActivation::BSplinesActivation(const int degree, const int grid_size)
    : degree(degree), grid_size(grid_size), x_min(0.f), x_max(1.f) {}

int BSplinesActivation::get_size() { return degree + grid_size; }

torch::Tensor BSplinesActivation::forward(const torch::Tensor &x) {
    const auto out = x.unsqueeze(-1);
    const auto i_s = torch::arange(-grid_size, degree, torch::TensorOptions().device(x.device()));
    return torch::movedim(b_splines(out, i_s, grid_size), -1, 1);
}

torch::Tensor BSplinesActivation::b_splines(
    const torch::Tensor &x, const torch::Tensor &curr_i_s, const int &curr_k) {
    if (curr_k == 0)
        return torch::logical_and(torch::le(knots(curr_i_s), x), torch::lt(x, knots(curr_i_s + 1)))
            .to(torch::kFloat);

    return b_splines(x, curr_i_s, curr_k - 1) * (x - knots(curr_i_s))
               / (knots(curr_i_s + curr_k) - knots(curr_i_s))
           + b_splines(x, curr_i_s + 1, curr_k - 1) * (knots(curr_i_s + curr_k + 1) - x)
                 / (knots(curr_i_s + curr_k + 1) - knots(curr_i_s + 1));
}

torch::Tensor BSplinesActivation::knots(const torch::Tensor &i) const {
    return i / degree * (x_max - x_min) + x_min;
}

// Linear KAN

LinearKAN::LinearKAN(
    int in_features, int out_features, const std::shared_ptr<ActivationFunction> &activation,
    const std::function<torch::Tensor(torch::Tensor)> &res_act_fun)
    : w_b(register_parameter("w_b", torch::ones({out_features, in_features}))),
      w_s(register_parameter("w_c", torch::ones({out_features, in_features}))),
      c(register_parameter("c", torch::ones({activation->get_size(), out_features, in_features}))),
      act(register_module("act", activation)), res_act_fun(res_act_fun) {

    torch::nn::init::xavier_normal_(w_b, 1e-3f);
    torch::nn::init::normal_(c.data(), 0.f, 1e-3f);
}

torch::Tensor LinearKAN::forward(const torch::Tensor &x) {
    return torch::sum(
        torch::einsum("boi,oi->boi", {torch::einsum("bai,aoi->boi", {act->forward(x), c}), w_s})
            + torch::einsum("bi,oi->boi", {res_act_fun(x), w_b}),
        2);
}
