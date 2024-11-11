//
// Created by samuel on 11/11/24.
//

#ifndef EVO_MOTION_LIQUID_H
#define EVO_MOTION_LIQUID_H

#include <torch/torch.h>

class LiquidCellModule final : public torch::nn::Module {
public:
    LiquidCellModule(const int &input_space, int neuron_number, int unfolding_steps);

    void reset_x_t();
    torch::Tensor gen_first_x_t(int batch_size);
    torch::Tensor get_x_t();
    void set_x_t(const torch::Tensor &new_x_t);

    torch::Tensor compute_step(const torch::Tensor &x_t_curr, const torch::Tensor &i_t);

    torch::Tensor forward(const torch::Tensor &state);
    torch::Tensor forward(const torch::Tensor &last_x_t, const torch::Tensor &state);

    void to(torch::Device device, bool non_blocking) override;

private:
    int steps;
    int neuron_number;

    torch::nn::Linear weight{nullptr};
    torch::nn::Linear recurrent_weight{nullptr};
    torch::Tensor bias;

    torch::Tensor a;
    torch::Tensor tau;

    torch::Tensor x_t;
};
#endif//EVO_MOTION_LIQUID_H
