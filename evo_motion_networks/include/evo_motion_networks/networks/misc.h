//
// Created by samuel on 26/11/24.
//

#ifndef MISC_H
#define MISC_H

#include <torch/torch.h>

class ExpModule final : public torch::nn::Module {
public:
    explicit ExpModule(float epsilon = 0.f);
    torch::Tensor forward(const torch::Tensor &input);

private:
    float epsilon;
};

class ClampModule final : public torch::nn::Module {
public:
    ClampModule(float min_value, float max_value);
    torch::Tensor forward(const torch::Tensor &input);
private:
    float min_value;
    float max_value;
};

#endif //MISC_H
