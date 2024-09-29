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
    const std::vector<int64_t> &action_space, int hidden_size, const int batch_size, float lr,
    const float gamma)
    : actor(std::make_shared<ActorModule>(state_space, action_space, hidden_size)),
      actor_optimizer(std::make_shared<torch::optim::Adam>(actor->parameters(), lr)),
      critic(std::make_shared<CriticModule>(state_space, hidden_size)),
      critic_optimizer(std::make_shared<torch::optim::Adam>(critic->parameters(), lr)),
      gamma(gamma), curr_device(torch::kCPU), batch_size(batch_size), episodes_buffer({{}}),
      episode_actor_loss(0.f), episode_critic_loss(0.f), curr_episode_step(0),
      last_episode_steps(0), curr_train_step(0L) {
    at::manual_seed(seed);
}

torch::Tensor ActorCritic::act(const torch::Tensor state, const float reward) {
    const auto [mu, sigma] = actor->forward(state);
    const auto [value] = critic->forward(state);

    auto action = truncated_normal_sample(mu, sigma, -1.f, 1.f);

    episodes_buffer.back().rewards_buffer.push_back(reward);
    episodes_buffer.back().mu_buffer.push_back(mu);
    episodes_buffer.back().sigma_buffer.push_back(sigma);
    episodes_buffer.back().value_buffer.push_back(value);
    episodes_buffer.back().actions_buffer.push_back(action);

    curr_episode_step++;

    return action;
}

void ActorCritic::train(
    const torch::Tensor &batched_actions, const torch::Tensor &batched_values,
    const torch::Tensor &batched_mus, const torch::Tensor &batched_sigmas,
    const torch::Tensor &batched_rewards) {
    const auto gamma_factor =
        torch::pow(
            gamma, torch::arange(batched_rewards.size(1), at::TensorOptions().device(curr_device)))
            .unsqueeze(0);

    auto returns = (batched_rewards * gamma_factor).flip({1}).cumsum(1).flip({1}) / gamma_factor;
    returns = (returns - returns.mean()) / (returns.std() + 1e-8);

    const auto prob =
        truncated_normal_pdf(batched_actions.detach(), batched_mus, batched_sigmas, -1.f, 1.f);
    const auto actor_loss = -torch::mean(
        torch::sum(torch::log(prob) * (returns - batched_values).detach().unsqueeze(-1), -1));
    const auto critic_loss = torch::smooth_l1_loss(batched_values, returns, at::Reduction::Mean);

    actor_optimizer->zero_grad();
    actor_loss.backward();
    actor_optimizer->step();

    critic_optimizer->zero_grad();
    critic_loss.backward();
    critic_optimizer->step();

    episode_actor_loss = actor_loss.cpu().item().toFloat();
    episode_critic_loss = critic_loss.cpu().item().toFloat();

    curr_train_step++;
}

/*float ActorCritic::get_exponential_entropy_factor() const {
    const auto lambda = -std::log(wanted_entropy_factor) / static_cast<float>(entropy_factor_steps);
    return first_entropy_factor * std::exp(-lambda * static_cast<float>(curr_train_step));
}*/

void ActorCritic::done(const float reward) {
    episodes_buffer.back().rewards_buffer.push_back(reward);
    episodes_buffer.back().rewards_buffer.erase(episodes_buffer.back().rewards_buffer.begin());

    if (actor->is_training() && static_cast<int>(episodes_buffer.size()) == batch_size) {
        int episode_max_step = 0;

        std::vector<torch::Tensor> actions_per_episode, values_per_episode, mus_per_episode,
            sigmas_per_episode, rewards_per_episode;

        for (const auto &[mu_buffer, sigma_buffer, value_buffer, rewards_buffer, actions_buffer]:
             episodes_buffer)
            episode_max_step = std::max(static_cast<int>(actions_buffer.size()), episode_max_step);

        for (const auto &[mu_buffer, sigma_buffer, value_buffer, rewards_buffer, actions_buffer]:
             episodes_buffer) {
            int pad = episode_max_step - static_cast<int>(actions_buffer.size());

            actions_per_episode.push_back(torch::pad(torch::stack(actions_buffer), {0, 0, 0, pad}));

            values_per_episode.push_back(torch::pad(torch::cat(value_buffer), {0, pad}));

            mus_per_episode.push_back(torch::pad(torch::stack(mu_buffer), {0, 0, 0, pad}));
            sigmas_per_episode.push_back(
                torch::pad(torch::stack(sigma_buffer), {0, 0, 0, pad}, "constant", 1.f));

            rewards_per_episode.push_back(torch::pad(
                torch::tensor(rewards_buffer, at::TensorOptions().device(curr_device)), {0, pad}));
        }

        train(
            torch::stack(actions_per_episode), torch::stack(values_per_episode),
            torch::stack(mus_per_episode), torch::stack(sigmas_per_episode),
            torch::stack(rewards_per_episode));

        episodes_buffer.clear();
    } else if (!actor->is_training()) {
        episodes_buffer.clear();
    }

    episodes_buffer.push_back({});

    last_episode_steps = curr_episode_step;
    curr_episode_step = 0;
}

void ActorCritic::save(const std::string &output_folder_path) {
    const std::filesystem::path path(output_folder_path);

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
    float actor_grad = 0.f, critic_grad = 0.f;

    if (curr_train_step != 0) {
        const auto actor_params = actor->parameters();
        for (const auto &p: actor_params) actor_grad += p.grad().norm().item().toFloat();
        actor_grad /= static_cast<float>(actor_params.size());

        const auto critic_params = critic->parameters();
        for (const auto &p: critic_params) critic_grad += p.grad().norm().item().toFloat();
        critic_grad /= static_cast<float>(critic_params.size());
    }

    return {
        {"actor_loss", episode_actor_loss},
        {"critic_loss", episode_critic_loss},
        {"actor_grad_mean", actor_grad},
        {"critic_grad_mean", critic_grad},
        {"episode_steps", last_episode_steps}};
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