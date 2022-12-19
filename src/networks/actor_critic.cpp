//
// Created by samuel on 19/12/22.
//

#include <torch/torch.h>

#include "./networks/actor_critic.h"

torch::Tensor ActorCritic::act(step step) {
    rewards_buffer.push_back(step.reward);

    auto response = networks->forward(step.state);
    results_buffer.push_back(response);

    auto action = response.mu + torch::randn(response.mu.sizes()) * response.sigma.sqrt();
    actions_buffer.push_back(action);

    return action;
}

void ActorCritic::train() {
    rewards_buffer.erase(rewards_buffer.begin());

    std::vector<torch::Tensor> mus_tmp;
    std::vector<torch::Tensor> sigmas_tmp;
    std::vector<torch::Tensor> values_tmp;

    for (auto [mu, sigma, value] : results_buffer) {
        mus_tmp.push_back(mu);
        sigmas_tmp.push_back(sigma);
        values_tmp.push_back(value);
    }
    auto actions = torch::cat(actions_buffer);
    auto values = torch::cat(values_tmp);
    auto mus = torch::cat(mus_tmp);
    auto sigmas = torch::cat(sigmas_tmp);

    auto rewards = torch::tensor(rewards_buffer);
    auto t_steps = torch::arange(int(rewards_buffer.size()));

    auto returns = rewards * torch::pow(gamma, t_steps);
    returns = returns.flip({0})
            .cumsum(0)
            .flip({0})
            / torch::pow(gamma, t_steps);
    returns = (returns - returns.mean()) / (returns.std() + 1e-8f);

    auto advantage = returns - values;

    auto prob = torch::exp(-0.5f * torch::pow((actions - mus) / sigmas, 2.f))
            / (sigmas * sqrt(2. * M_PI));
    auto log_prob = torch::log(prob);

    auto actor_loss = -log_prob * advantage.detach();

    auto critic_loss = torch::smooth_l1_loss(values, returns.detach(), at::Reduction::None);

    auto loss = (actor_loss + critic_loss).sum(0);

    optimizer.zero_grad();
    loss.backward();
    optimizer.step();

    std::cout << "policy " << actor_loss.sum().item() << " critic " << critic_loss.sum().item() << std::endl;
}

void ActorCritic::done(step step) {
    rewards_buffer.push_back(step.reward);

    train();

    results_buffer.clear();
    rewards_buffer.clear();
    actions_buffer.clear();
}

ActorCritic::ActorCritic(
        const std::vector<int64_t>& state_space,
        const std::vector<int64_t>& action_space,
        int hidden_size,
        float lr
        ) :
        gamma(0.95),
        networks(std::make_shared<a2c_networks>(state_space, action_space, hidden_size)),
        rewards_buffer(),
        results_buffer(),
        actions_buffer(),
        optimizer(networks->parameters(), lr),
        rng(1234) {

}

a2c_networks::a2c_networks(std::vector<int64_t> state_space, std::vector<int64_t> action_space,
                           int hidden_size) {
    head = register_module("head", torch::nn::Linear(state_space[0], hidden_size));

    critic = register_module("critic", torch::nn::Linear(hidden_size, 1));

    mu = register_module("mu", torch::nn::Linear(hidden_size, action_space[0]));
    sigma = register_module("sigma", torch::nn::Linear(hidden_size, action_space[0]));

}

a2c_response a2c_networks::forward(const torch::Tensor& state) {
    auto head_out = torch::relu(head->forward(state));

    return {
        torch::tanh(mu->forward(head_out)),
        torch::softplus(sigma->forward(head_out)),
        critic->forward(head_out)
    };
}

