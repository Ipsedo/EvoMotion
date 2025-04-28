//
// Created by samuel on 21/04/25.
//

#include <evo_motion_networks/functions.h>
#include <evo_motion_networks/init.h>
#include <evo_motion_networks/networks/kan.h>
#include <evo_motion_networks/networks/q_net.h>

QNetworkKanModule::QNetworkKanModule(
    const std::vector<int64_t> &state_space, const std::vector<int64_t> &action_space,
    int hidden_size, int poly_degree, int grid_size)
    : state_space(state_space), action_space(action_space), hidden_size(hidden_size),
      poly_degree(poly_degree), grid_size(grid_size),
      q_network(register_module(
          "q_network",
          torch::nn::Sequential(
              std::make_shared<LinearKAN>(
                  state_space[0] + action_space[0], hidden_size,
                  std::make_shared<BSplinesActivation>(poly_degree, grid_size),
                  torch::nn::functional::mish),
              std::make_shared<LinearKAN>(
                  hidden_size, hidden_size,
                  std::make_shared<BSplinesActivation>(poly_degree, grid_size),
                  torch::nn::functional::mish),
              std::make_shared<LinearKAN>(
                  hidden_size, 1, std::make_shared<BSplinesActivation>(poly_degree, grid_size),
                  torch::nn::functional::mish)))) {

    apply(init_weights);
}

critic_response
QNetworkKanModule::forward(const torch::Tensor &state, const torch::Tensor &action) {
    bool only_one = false;

    torch::Tensor in_q_net = torch::cat({state, action}, -1);

    if (in_q_net.sizes().size() == 1) {
        in_q_net = in_q_net.unsqueeze(0);
        only_one = true;
    }

    auto q_value = q_network->forward(in_q_net);

    if (only_one) q_value = q_value.squeeze(0);

    return {q_value};
}

std::shared_ptr<AbstractQNetwork> QNetworkKanModule::clone() {
    auto clone = std::make_shared<QNetworkKanModule>(
        state_space, action_space, hidden_size, poly_degree, grid_size);
    hard_update(clone, std::make_shared<QNetworkKanModule>(*this));
    return clone;
}