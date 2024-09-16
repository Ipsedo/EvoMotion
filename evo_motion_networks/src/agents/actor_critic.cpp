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

// shared network

ActorCriticModule::ActorCriticModule(
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

a2c_response ActorCriticModule::forward(const torch::Tensor &state) {
    auto head_out = head->forward(state.unsqueeze(0));

    return {
        mu->forward(head_out).squeeze(0), sigma->forward(head_out).squeeze(0),
        critic->forward(head_out)};
}

// separated networks - actor

ActorModule::ActorModule(std::vector<int64_t> state_space, std::vector<int64_t> action_space, int hidden_size) {
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

    return {
        mu->forward(head_out).squeeze(0), sigma->forward(head_out).squeeze(0)};
}

// separated networks - critic

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
            torch::nn::Linear(hidden_size, hidden_size), torch::nn::Mish(),
            torch::nn::LayerNorm(
                torch::nn::LayerNormOptions({hidden_size}).elementwise_affine(true).eps(1e-5)),

            torch::nn::Linear(hidden_size, 1)));
}

critic_response CriticModule::forward(const torch::Tensor &state) {
    return {critic->forward(state.unsqueeze(0)).squeeze(0)};
}


/*
 * Agent class
 */

ActorCritic::ActorCritic(
    const int seed, const std::vector<int64_t> &state_space,
    const std::vector<int64_t> &action_space, int hidden_size, float lr)
    : /*actor_critic(std::make_shared<ActorCriticModule>(state_space, action_space, hidden_size)),
      optimizer(std::make_shared<torch::optim::Adam>(actor_critic->parameters(), lr)),*/
      actor(std::make_shared<ActorModule>(state_space, action_space, hidden_size)),
      actor_optimizer(std::make_shared<torch::optim::Adam>(actor->parameters(), lr)),
      critic(std::make_shared<CriticModule>(state_space, hidden_size)),
      critic_optimizer(std::make_shared<torch::optim::Adam>(actor->parameters(), lr)),
      gamma(0.99f), first_entropy_factor(5e-2), wanted_entropy_factor(1e-3), entropy_factor_steps(1L << 12),
      curr_device(torch::kCPU), episode_policy_loss(0.f), episode_policy_entropy(0.f), episode_critic_loss(0.f),
      curr_step(0L) {
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
    const auto gamma_factor = torch::pow(
        gamma, torch::arange(
            static_cast<int>(rewards_buffer.size()), at::TensorOptions().device(curr_device)));

    auto returns = (rewards * gamma_factor).flip({0}).cumsum(0).flip({0}) / gamma_factor;
    returns = (returns - returns.mean()) / (returns.std() + 1e-8);

    const auto prob = truncated_normal_pdf(actions.detach(), mus, sigmas, -1.f, 1.f);
    const auto policy_loss = torch::log(prob) * (returns - values).detach().unsqueeze(-1);
    const auto policy_entropy = truncated_normal_entropy(mus, sigmas, -1.f, 1.f);
    const auto entropy_factor = get_exponential_entropy_factor();
    const auto actor_loss = -torch::mean(torch::sum(policy_loss + entropy_factor * policy_entropy, -1));

    const auto critic_loss = torch::smooth_l1_loss(values, returns, at::Reduction::Mean);

    /*const auto loss = actor_loss + critic_loss;

    optimizer->zero_grad();
    loss.backward();
    optimizer->step();*/

    actor_optimizer->zero_grad();
    actor_loss.backward();
    actor_optimizer->step();

    critic_optimizer->zero_grad();
    critic_loss.backward();
    critic_optimizer->step();

    episode_policy_loss = -policy_loss.sum(-1).mean().cpu().detach().item().toFloat();
    episode_policy_entropy = -entropy_factor * policy_entropy.sum(-1).mean().cpu().detach().item().toFloat();
    episode_critic_loss = critic_loss.cpu().detach().item().toFloat();

    curr_step++;
}

float ActorCritic::get_exponential_entropy_factor() const {
    const auto lambda = -std::log(wanted_entropy_factor) / static_cast<float>(entropy_factor_steps);
    return first_entropy_factor * std::exp(-lambda * static_cast<float>(curr_step));
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

    /*const auto model_file = path / "actor_critic.th";
    const auto optimizer_file = path / "optimizer.th";

    torch::serialize::OutputArchive model_archive;
    torch::serialize::OutputArchive optimizer_archive;

    // Save networks
    actor_critic->save(model_archive);
    optimizer->save(optimizer_archive);

    model_archive.save_to(model_file);
    optimizer_archive.save_to(optimizer_file);*/

    // actor
    const auto actor_model_file = path / "actor.th";
    const auto actor_optimizer_file = path / "actor_optimizer.th";

    torch::serialize::OutputArchive actor_model_archive;
    torch::serialize::OutputArchive actor_optimizer_archive;

    actor->save(actor_model_archive);
    actor_optimizer->save(actor_optimizer_archive);

    actor_model_archive.save_to(actor_model_file);
    actor_optimizer_archive.save_to(actor_optimizer_file);

    // critic
    const auto critic_model_file = path / "critic.th";
    const auto critic_optimizer_file = path / "critic_optimizer.th";

    torch::serialize::OutputArchive critic_model_archive;
    torch::serialize::OutputArchive critic_optimizer_archive;

    critic->save(critic_model_archive);
    critic_optimizer->save(critic_optimizer_archive);

    critic_model_archive.save_to(critic_model_file);
    critic_optimizer_archive.save_to(critic_optimizer_file);
}

void ActorCritic::load(const std::string &input_folder_path) {
    const std::filesystem::path path(input_folder_path);

    /*const auto model_file = path / "actor_critic.th";
    const auto optimizer_file = path / "optimizer.th";

    // load
    torch::serialize::InputArchive model_archive;
    torch::serialize::InputArchive optimizer_archive;

    model_archive.load_from(model_file);
    optimizer_archive.load_from(optimizer_file);

    actor_critic->load(model_archive);
    optimizer->load(optimizer_archive);*/

    // actor
    const auto actor_model_file = path / "actor.th";
    const auto actor_optimizer_file = path / "actor_optimizer.th";

    torch::serialize::InputArchive actor_model_archive;
    torch::serialize::InputArchive actor_optimizer_archive;

    actor_model_archive.load_from(actor_model_file);
    actor_optimizer_archive.load_from(actor_optimizer_file);

    actor->load(actor_model_archive);
    actor_optimizer->load(actor_optimizer_archive);

    // critic
    const auto critic_model_file = path / "critic.th";
    const auto critic_optimizer_file = path / "critic_optimizer.th";

    torch::serialize::InputArchive critic_model_archive;
    torch::serialize::InputArchive critic_optimizer_archive;

    critic_model_archive.load_from(critic_model_file);
    critic_optimizer_archive.load_from(critic_optimizer_file);

    critic->load(critic_model_archive);
    critic_optimizer->load(critic_optimizer_archive);
}

std::map<std::string, float> ActorCritic::get_metrics() {
    float actor_grad = 0.f;
    const auto actor_params = actor->parameters();
    for (const auto &p: actor_params) actor_grad += p.grad().norm().item().toFloat();
    actor_grad /= static_cast<float>(actor_params.size());

    float critic_grad = 0.f;
    const auto critic_params = critic->parameters();
    for (const auto &p: critic_params) critic_grad += p.grad().norm().item().toFloat();
    critic_grad /= static_cast<float>(critic_params.size());

    return {{"policy_loss", episode_policy_loss}, {"policy_entropy", episode_policy_entropy},
            {"critic_loss", episode_critic_loss}, {"entropy_factor", get_exponential_entropy_factor()},
            {"actor_grad_mean", actor_grad}, {"critic_grad_mean", critic_grad}};
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