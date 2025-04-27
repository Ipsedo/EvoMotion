//
// Created by samuel on 21/04/25.
//

#ifndef KAN_H
#define KAN_H

#include <torch/torch.h>

// Actiation functions

class ActivationFunction : public torch::nn::Module {
public:
    virtual int get_size() = 0;
    virtual torch::Tensor forward(const torch::Tensor &x) = 0;
};

class PolyCoefActivation : public ActivationFunction {
public:
    PolyCoefActivation(int n, const torch::Tensor &coefficients, float scale_coefficient = 10.f);

    int get_size() override;
    torch::Tensor forward(const torch::Tensor &x) override;

private:
    int n;
    torch::Tensor coefficients;
    torch::Tensor exponent;
    torch::Tensor scale_coefficients;
};

class HermiteActivation final : public PolyCoefActivation {
public:
    explicit HermiteActivation(int n);

private:
    torch::Tensor scale_coefficients;

    static torch::Tensor hermite_coef(int n);
    static float factorial(int n);
};

// Linear

class LinearKAN final : public torch::nn::Module {
public:
    LinearKAN(
        int in_features, int out_features, const std::shared_ptr<ActivationFunction> &activation,
        const std::function<torch::Tensor(torch::Tensor)> &res_act_fun);

    torch::Tensor forward(const torch::Tensor &x);

private:
    torch::Tensor w_b;
    torch::Tensor w_s;
    torch::Tensor c;

    std::shared_ptr<ActivationFunction> act;
    std::function<torch::Tensor(torch::Tensor)> res_act_fun;
};

#endif//KAN_H
