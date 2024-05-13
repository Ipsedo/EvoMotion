//
// Created by samuel on 19/12/22.
//

#include "./actor_critic.h"

#include <algorithm>
#include <filesystem>
#include <numeric>

#include <torch/torch.h>

#include "../functions.h"

/*
 * torch Module
 */

// actor

ActorModule::ActorModule(
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


}

actor_response ActorModule::forward(const torch::Tensor &state) {
    auto head_out = head->forward(state.unsqueeze(0));

    return {mu->forward(head_out).squeeze(0), sigma->forward(head_out).squeeze(0)};
}

// critic

CriticModule::CriticModule(std::vector<int64_t> state_space, int hidden_size) {
    critic = register_module(
        "critic",
        torch::nn::Sequential(
            torch::nn::Linear(state_space[0], hidden_size), torch::nn::Mish(),
            torch::nn::LayerNorm(
                torch::nn::LayerNormOptions({hidden_size}).elementwise_affine(true).eps(1e-5)),

            torch::nn::Linear(hidden_size, hidden_size), torch::nn::Mish(),
            torch::nn::LayerNorm(
                torch::nn::LayerNormOptions({hidden_size}).elementwise_affine(true).eps(1e-5)),

            torch::nn::Linear(hidden_size, 1)));
}

critic_response CriticModule::forward(const torch::Tensor &state) {
    return {critic->forward(state).squeeze(0)};
}

/*
 * Agent class
 */

ActorCritic::ActorCritic(
    const int seed, const std::vector<int64_t> &state_space,
    const std::vector<int64_t> &action_space, int hidden_size, float lr)
: actor(std::make_shared<ActorModule>(state_space, action_space, hidden_size)),
      actor_optimizer(std::make_shared<torch::optim::Adam>(actor->parameters(), lr)),
      critic(std::make_shared<CriticModule>(state_space, hidden_size)),
      critic_optimizer(std::make_shared<torch::optim::Adam>(critic->parameters(), lr)),
      actor_loss_factor(1.f), critic_loss_factor(1.f), curr_device(torch::kCPU), gamma(0.99f),
      episode_actor_loss(0.f), episode_critic_loss(0.f) {
    at::manual_seed(seed);
}

torch::Tensor ActorCritic::act(const torch::Tensor state, const float reward) {
    const auto [mu, sigma] = actor->forward(state);
    const auto [value] = critic->forward(state);

    const a2c_response response = {mu, sigma, value};

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
    const auto t_steps = torch::arange(
        static_cast<int>(rewards_buffer.size()), at::TensorOptions().device(curr_device));

    auto returns = rewards * torch::pow(gamma, t_steps);
    returns = returns.flip({0}).cumsum(0).flip({0}) / torch::pow(gamma, t_steps);
    returns = (returns - returns.mean()) / (returns.std() + 1e-8f);

    const auto advantage = torch::smooth_l1_loss(returns, values, at::Reduction::None);

    const auto prob = truncated_normal_pdf(actions.detach(), mus, sigmas, -1.f, 1.f);
    const auto actor_loss =
        -torch::log(prob) * advantage.detach().unsqueeze(-1) * actor_loss_factor;

    const auto critic_loss =
        torch::smooth_l1_loss(values, returns.detach(), at::Reduction::None) * critic_loss_factor;

    const auto loss =
        torch::sum(actor_loss_factor * actor_loss + critic_loss_factor * critic_loss.unsqueeze(-1));

    actor_optimizer->zero_grad();
    actor_loss.sum().backward();
    actor_optimizer->step();

    critic_optimizer->zero_grad();
    critic_loss.sum().backward();
    critic_optimizer->step();

    episode_actor_loss = actor_loss.sum().item().toFloat();
    episode_critic_loss = critic_loss.sum().item().toFloat();
}

void ActorCritic::done(const float reward) {
    rewards_buffer.push_back(reward);

    if (actor->is_training()) train();

    results_buffer.clear();
    rewards_buffer.clear();
    actions_buffer.clear();
}

void ActorCritic::save(const std::string &output_folder_path) {
    const std::filesystem::path path(output_folder_path);

    const auto actor_file = path / "actor.th";
    const auto actor_optimizer_file = path / "actor_optimizer.th";

    const auto critic_file = path / "critic.th";
    const auto critic_optimizer_file = path / "critic_optimizer.th";

    torch::serialize::OutputArchive actor_archive;
    torch::serialize::OutputArchive actor_optimizer_archive;

    torch::serialize::OutputArchive critic_archive;
    torch::serialize::OutputArchive critic_optimizer_archive;

    // Save networks
    actor->save(actor_archive);
    actor_optimizer->save(actor_optimizer_archive);

    critic->save(critic_archive);
    critic_optimizer->save(critic_optimizer_archive);

    actor_archive.save_to(actor_file);
    actor_optimizer_archive.save_to(actor_optimizer_file);

    critic_archive.save_to(critic_file);
    critic_optimizer_archive.save_to(critic_optimizer_file);

}

void ActorCritic::load(const std::string &input_folder_path) {
    const std::filesystem::path path(input_folder_path);

    const auto actor_file = path / "actor.th";
    const auto actor_optimizer_file = path / "actor_optimizer.th";

    const auto critic_file = path / "critic.th";
    const auto critic_optimizer_file = path / "critic_optimizer.th";

    // load
    torch::serialize::InputArchive actor_archive;
    torch::serialize::InputArchive actor_optimizer_archive;

    torch::serialize::InputArchive critic_archive;
    torch::serialize::InputArchive critic_optimizer_archive;

    actor_archive.load_from(actor_file);
    actor_optimizer_archive.load_from(actor_optimizer_file);

    critic_archive.load_from(critic_file);
    critic_optimizer_archive.load_from(critic_optimizer_file);

    actor->load(actor_archive);
    actor_optimizer->load(actor_optimizer_archive);

    critic->load(critic_archive);
    critic_optimizer->load(critic_optimizer_archive);
}

std::map<std::string, float> ActorCritic::get_metrics() {
    return {{"actor_loss", episode_actor_loss}, {"critic_loss", episode_critic_loss}};
}

void ActorCritic::to(const torch::DeviceType device) {
    curr_device = device;
    actor->to(curr_device);
    critic->to(curr_device);
}

void ActorCritic::set_eval(const bool eval) {
    if (eval) {
        actor->eval();
        critic->eval();
    } else {
        actor->train();
        critic->train();
    }
}

int ActorCritic::count_parameters() {
    int count = 0;
    for (const auto &p: actor->parameters()) {
        auto sizes = p.sizes();
        count += std::reduce(sizes.begin(), sizes.end(), 1, std::multiplies<>());
    }
    for (const auto &p: critic->parameters()) {
        auto sizes = p.sizes();
        count += std::reduce(sizes.begin(), sizes.end(), 1, std::multiplies<>());
    }
    return count;
}

float ActorCritic::grad_norm_mean() {
    float grad_sum = 0.f;

    const auto actor_params = actor->parameters();
    const auto critic_params = critic->parameters();

    for (const auto &p: actor_params) { grad_sum += p.grad().norm().item().toFloat(); }
    for (const auto &p: critic_params) { grad_sum += p.grad().norm().item().toFloat(); }

    return grad_sum
           / (static_cast<float>(actor_params.size()) + static_cast<float>(critic_params.size()));
}