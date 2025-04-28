//
// Created by samuel on 21/04/25.
//

#include <evo_motion_networks/init.h>
#include <evo_motion_networks/networks/actor.h>
#include <evo_motion_networks/networks/kan.h>

ActorKanModule::ActorKanModule(
    const std::vector<int64_t> &state_space, const std::vector<int64_t> &action_space,
    int hidden_size, int poly_degree, int grid_size)
    : head(register_module(
          "head", torch::nn::Sequential(
                      std::make_shared<LinearKAN>(
                          state_space[0], hidden_size,
                          std::make_shared<BSplinesActivation>(poly_degree, grid_size),
                          torch::nn::functional::mish),

                      std::make_shared<LinearKAN>(
                          hidden_size, hidden_size,
                          std::make_shared<BSplinesActivation>(poly_degree, grid_size),
                          torch::nn::functional::mish)))),
      mu(register_module(
          "mu", torch::nn::Sequential(
                    std::make_shared<LinearKAN>(
                        hidden_size, action_space[0],
                        std::make_shared<BSplinesActivation>(poly_degree, grid_size),
                        torch::nn::functional::mish),
                    torch::nn::Tanh()))),
      sigma(register_module(
          "sigma", torch::nn::Sequential(
                       std::make_shared<LinearKAN>(
                           hidden_size, action_space[0],
                           std::make_shared<BSplinesActivation>(poly_degree, grid_size),
                           torch::nn::functional::mish),
                       torch::nn::Softplus()))) {

    apply(init_weights);
}

actor_response ActorKanModule::forward(const torch::Tensor &state) {
    bool only_one = false;
    torch::Tensor input_head = state;

    if (input_head.sizes().size() == 1) {
        input_head = input_head.unsqueeze(0);
        only_one = true;
    }

    auto head_out = head->forward(input_head);
    auto out_mu = mu->forward(head_out);
    auto out_sigma = sigma->forward(head_out);

    if (only_one) {
        out_mu = out_mu.squeeze(0);
        out_sigma = out_sigma.squeeze(0);
    }

    return {out_mu, out_sigma};
}
