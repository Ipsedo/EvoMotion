//
// Created by samuel on 21/04/25.
//

#include <evo_motion_networks/init.h>
#include <evo_motion_networks/networks/critic.h>
#include <evo_motion_networks/networks/kan.h>

CriticKanModule::CriticKanModule(
    const std::vector<int64_t> &state_space, int hidden_size, int poly_degree)
    : critic(register_module(
          "critic",
          torch::nn::Sequential(
              std::make_shared<LinearKAN>(
                  state_space[0], hidden_size, std::make_shared<HermiteActivation>(poly_degree),
                  torch::nn::functional::mish),
              std::make_shared<LinearKAN>(
                  hidden_size, hidden_size, std::make_shared<HermiteActivation>(poly_degree),
                  torch::nn::functional::mish),
              std::make_shared<LinearKAN>(
                  hidden_size, 1, std::make_shared<HermiteActivation>(poly_degree),
                  torch::nn::functional::mish)))) {

    apply(init_weights);
}

critic_response CriticKanModule::forward(const torch::Tensor &state) {
    bool only_one = false;
    torch::Tensor in_critic = state;

    if (in_critic.sizes().size() == 1) {
        in_critic = in_critic.unsqueeze(0);
        only_one = true;
    }

    auto out_critic = critic->forward(in_critic);

    if (only_one) { out_critic = out_critic.squeeze(0); }

    return {out_critic};
}
