//
// Created by samuel on 04/05/24.
//

#include "./init.h"

void init_weights(torch::nn::Module &module) {
    if (typeid(module) == typeid(torch::nn::Linear)) {
        auto lin = *dynamic_cast<torch::nn::Linear *>(&module);

        torch::nn::init::xavier_normal_(lin->weight, 1e-3f);

        if (lin->options.bias()) torch::nn::init::normal_(lin->bias, 0.f, 1e-3f);

    } else if (typeid(module) == typeid(torch::nn::LayerNorm)) {
        auto ln = *dynamic_cast<torch::nn::LayerNorm *>(&module);

        if (ln->options.elementwise_affine()) {
            torch::nn::init::ones_(ln->weight);
            torch::nn::init::zeros_(ln->bias);
        }
    }
}
