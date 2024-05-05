//
// Created by samuel on 19/12/22.
//

#include "./actor_critic.h"

#include <filesystem>

#include <torch/torch.h>

#include "../functions.h"

ActorCritic::ActorCritic(
    const int seed, const std::vector<int64_t> &state_space,
    const std::vector<int64_t> &action_space,
    int hidden_size, float lr)
    : curr_device(torch::kCPU), gamma(0.99f),
      networks(std::make_shared<a2c_networks>(state_space, action_space, hidden_size)),
      optimizer(std::make_shared<torch::optim::Adam>(networks->parameters(), lr)),
      episode_actor_loss(0.f), episode_critic_loss(0.f), actor_loss_factor(1.f),
      critic_loss_factor(1.f) {
    at::manual_seed(seed);
}

torch::Tensor ActorCritic::act(const torch::Tensor state, const float reward) {
    const auto response = networks->forward(state);
    auto action = truncated_normal_sample(response.mu, response.sigma, -1.f, 1.f);

    rewards_buffer.push_back(reward);
    results_buffer.push_back(response);
    actions_buffer.push_back(action);

    return action;
}

void ActorCritic::train() {
    rewards_buffer.erase(rewards_buffer.begin());

    std::vector<torch::Tensor> mus_tmp;
    std::vector<torch::Tensor> sigmas_tmp;
    std::vector<torch::Tensor> values_tmp;

    for (const auto &[mu, sigma, value]: results_buffer) {
        mus_tmp.push_back(mu);
        sigmas_tmp.push_back(sigma);
        values_tmp.push_back(value);
    }

    const auto actions = torch::stack(actions_buffer);
    const auto values = torch::cat(values_tmp);
    const auto mus = torch::stack(mus_tmp);
    const auto sigmas = torch::stack(sigmas_tmp);

    const auto rewards = torch::tensor(rewards_buffer, at::TensorOptions().device(curr_device));
    const auto t_steps =
        torch::arange(
            static_cast<int>(rewards_buffer.size()), at::TensorOptions().device(curr_device));

    auto returns = rewards * torch::pow(gamma, t_steps);
    returns = returns.flip({0}).cumsum(0).flip({0}) / torch::pow(gamma, t_steps);
    returns = (returns - returns.mean()) / (returns.std() + 1e-8f);

    const auto advantage = torch::smooth_l1_loss(returns, values, at::Reduction::None);

    const auto prob = truncated_normal_pdf(actions.detach(), mus, sigmas, -1.f, 1.f);
    const auto log_prob = torch::log(prob);

    const auto actor_loss = -log_prob * advantage.detach().unsqueeze(-1);

    const auto critic_loss = torch::smooth_l1_loss(values, returns.detach(), at::Reduction::None);

    const auto loss =
        (actor_loss_factor * actor_loss + critic_loss_factor * critic_loss.unsqueeze(-1)).sum();

    optimizer->zero_grad();
    loss.backward();
    optimizer->step();

    episode_actor_loss = actor_loss.sum().item().toFloat();
    episode_critic_loss = critic_loss.sum().item().toFloat();
}

void ActorCritic::done(const float reward) {
    rewards_buffer.push_back(reward);

    if (networks->is_training()) train();

    results_buffer.clear();
    rewards_buffer.clear();
    actions_buffer.clear();
}

void ActorCritic::save(const std::string &output_folder_path) {
    const std::filesystem::path path(output_folder_path);

    const auto networks_file = path / "networks.th";
    const auto optimizer_file = path / "optimizer.th";

    torch::serialize::OutputArchive networks_archive;
    torch::serialize::OutputArchive optimizer_archive;

    // Save networks
    networks->save(networks_archive);
    optimizer->save(optimizer_archive);

    networks_archive.save_to(networks_file);
    optimizer_archive.save_to(optimizer_file);
}

void ActorCritic::load(const std::string &input_folder_path) {
    const std::filesystem::path path(input_folder_path);

    const auto networks_file = path / "networks.th";
    const auto optimizer_file = path / "optimizer.th";

    torch::serialize::InputArchive networks_archive;
    torch::serialize::InputArchive optimizer_archive;

    networks_archive.load_from(networks_file);
    optimizer_archive.load_from(optimizer_file);

    networks->load(networks_archive);
    optimizer->load(optimizer_archive);
}

std::map<std::string, float> ActorCritic::get_metrics() {
    return {{"actor_loss", episode_actor_loss}, {"critic_loss", episode_critic_loss}};
}

void ActorCritic::to(const torch::DeviceType device) {
    curr_device = device;
    networks->to(curr_device);
}

void ActorCritic::set_eval(const bool eval) {
    if (eval) networks->eval();
    else networks->train();
}

/*
 * torch Module
 */

a2c_networks::a2c_networks(
    std::vector<int64_t> state_space, std::vector<int64_t> action_space, int hidden_size) {
    head = register_module(
        "head",
        torch::nn::Sequential(
            torch::nn::Linear(state_space[0], hidden_size), torch::nn::Mish(),
            torch::nn::LayerNorm(
                torch::nn::LayerNormOptions({hidden_size}).elementwise_affine(true).eps(1e-5)),
            torch::nn::Linear(hidden_size, hidden_size), torch::nn::Mish(),
            torch::nn::LayerNorm(
                torch::nn::LayerNormOptions({hidden_size}).elementwise_affine(true).eps(1e-5))));

    mu = register_module(
        "mu",
        torch::nn::Sequential(
            torch::nn::Linear(hidden_size, hidden_size), torch::nn::Mish(),
            torch::nn::LayerNorm(
                torch::nn::LayerNormOptions({hidden_size}).elementwise_affine(true).eps(1e-5)),
            torch::nn::Linear(hidden_size, action_space[0]), torch::nn::Tanh()));

    sigma = register_module(
        "sigma",
        torch::nn::Sequential(
            torch::nn::Linear(hidden_size, hidden_size), torch::nn::Mish(),
            torch::nn::LayerNorm(
                torch::nn::LayerNormOptions({hidden_size}).elementwise_affine(true).eps(1e-5)),
            torch::nn::Linear(hidden_size, action_space[0]), torch::nn::Softplus()));

    critic = register_module(
        "critic",
        torch::nn::Sequential(
            torch::nn::Linear(hidden_size, hidden_size), torch::nn::Mish(),
            torch::nn::LayerNorm(
                torch::nn::LayerNormOptions({hidden_size}).elementwise_affine(true).eps(1e-5)),
            torch::nn::Linear(hidden_size, 1)));
}

a2c_response a2c_networks::forward(const torch::Tensor &state) {
    auto head_out = head->forward(state.unsqueeze(0));

    return {
        mu->forward(head_out).squeeze(0), sigma->forward(head_out).squeeze(0),
        critic->forward(head_out).squeeze(0)};
}
