//
// Created by samuel on 08/11/24.
//

#include "./soft_actor_critic.h"

#include <filesystem>

#include "../functions.h"

/*
 * Torch module
 */

QNetwork::QNetwork(
    std::vector<int64_t> state_space, std::vector<int64_t> action_space, int hidden_size) {
    q_network = register_module(
        "q_network",
        torch::nn::Sequential(
            torch::nn::Linear(state_space[0] + action_space[0], hidden_size), torch::nn::Mish(),
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

critic_response QNetwork::forward(const torch::Tensor &state, const torch::Tensor &action) {
    return {q_network->forward(torch::cat({state, action}, 0).unsqueeze(0)).squeeze(0)};
}

// Entropy Parameter

EntropyParameter::EntropyParameter() { register_parameter("log_alpha", torch::zeros({1})); }

torch::Tensor EntropyParameter::log_alpha() { return named_parameters()["log_alpha"]; }

torch::Tensor EntropyParameter::alpha() { return log_alpha().exp(); }

/*
 * Agents
 */

SoftActorCritic::SoftActorCritic(
    int seed, const std::vector<int64_t> &state_space, const std::vector<int64_t> &action_space,
    int hidden_size, int batch_size, float lr, float gamma, float tau)
    : actor(std::make_shared<ActorModule>(state_space, action_space, hidden_size)),
      critic_1(std::make_shared<QNetwork>(state_space, action_space, hidden_size)),
      critic_2(std::make_shared<QNetwork>(state_space, action_space, hidden_size)),
      target_critic_1(std::make_shared<QNetwork>(state_space, action_space, hidden_size)),
      target_critic_2(std::make_shared<QNetwork>(state_space, action_space, hidden_size)),
      actor_optimizer(std::make_shared<torch::optim::Adam>(actor->parameters(), lr)),
      critic_1_optimizer(std::make_shared<torch::optim::Adam>(critic_1->parameters(), lr)),
      critic_2_optimizer(std::make_shared<torch::optim::Adam>(critic_2->parameters(), lr)),
      target_entropy(-static_cast<float>(action_space[0])), entropy_parameter(),
      entropy_optimizer(entropy_parameter.parameters(), lr), curr_device(torch::kCPU), gamma(gamma),
      tau(tau), batch_size(batch_size), episodes_buffer({{}}), curr_episode_step(0),
      curr_train_step(0L), actor_loss_meter("actor", 16), critic_1_loss_meter("critic_1", 16),
      critic_2_loss_meter("critic_2", 16), entropy_loss_meter("entropy", 16),
      episode_steps_meter("steps", 16) {
    at::manual_seed(seed);

    for (auto n_p: critic_1->named_parameters()) {
        const auto &name = n_p.key();
        const auto &param = n_p.value();
        target_critic_1->named_parameters()[name].data().copy_(param.data());
    }

    for (auto n_p: critic_2->named_parameters()) {
        const auto &name = n_p.key();
        const auto &param = n_p.value();
        target_critic_2->named_parameters()[name].data().copy_(param.data());
    }
}

torch::Tensor SoftActorCritic::act(torch::Tensor state, float reward) {
    const auto [mu, sigma] = actor->forward(state);
    const auto action = truncated_normal_sample(mu, sigma, -1.f, 1.f);

    const auto [q_value_1] = critic_1->forward(state, action.detach());
    const auto [q_value_2] = critic_2->forward(state, action.detach());

    const auto [target_q_value_1] = target_critic_1->forward(state, action.detach());
    const auto [target_q_value_2] = target_critic_2->forward(state, action.detach());

    episodes_buffer.back().rewards_buffer.push_back(reward);
    episodes_buffer.back().mu_buffer.push_back(mu);
    episodes_buffer.back().sigma_buffer.push_back(sigma);
    episodes_buffer.back().q_value_1_buffer.push_back(q_value_1);
    episodes_buffer.back().q_value_2_buffer.push_back(q_value_2);
    episodes_buffer.back().target_q_value_1_buffer.push_back(target_q_value_1);
    episodes_buffer.back().target_q_value_2_buffer.push_back(target_q_value_2);
    episodes_buffer.back().actions_buffer.push_back(action);
    episodes_buffer.back().done_buffer.push_back(0.f);

    curr_episode_step++;

    return action;
}

void SoftActorCritic::train(
    const torch::Tensor &batched_actions, const torch::Tensor &batched_q_values_1,
    const torch::Tensor &batched_q_values_2, const torch::Tensor &batched_target_q_values_1,
    const torch::Tensor &batched_target_q_values_2, const torch::Tensor &batched_mus,
    const torch::Tensor &batched_sigmas, const torch::Tensor &batched_rewards,
    const torch::Tensor &batched_done) {

    const auto log_prob =
        truncated_normal_pdf(batched_actions, batched_mus, batched_sigmas, -1.f, 1.f);

    const auto curr_log_prob = torch::slice(log_prob, 1, 0, log_prob.size(1) - 1);
    const auto next_log_prob = torch::slice(log_prob, 1, 1);

    const auto next_target_q_value_1 = torch::slice(batched_target_q_values_1, 1, 1);
    const auto next_target_q_value_2 = torch::slice(batched_target_q_values_2, 1, 1);

    auto target_q_values = (batched_rewards
                            + gamma * (1.f - batched_done)
                                  * (torch::min(next_target_q_value_1, next_target_q_value_2)
                                     - entropy_parameter.alpha() * next_log_prob.sum(-1)))
                               .detach();
    target_q_values = (target_q_values - target_q_values.mean()) / (target_q_values.std() + 1e-8);

    // critic 1
    const auto critic_1_loss =
        torch::mse_loss(batched_q_values_1, target_q_values, at::Reduction::Mean);

    critic_1_optimizer->zero_grad();
    critic_1_loss.backward();
    critic_1_optimizer->step();

    // critic 2
    const auto critic_2_loss =
        torch::mse_loss(batched_q_values_2, target_q_values, at::Reduction::Mean);

    critic_2_optimizer->zero_grad();
    critic_2_loss.backward();
    critic_2_optimizer->step();

    // policy
    const auto q_value = torch::min(batched_q_values_1, batched_q_values_2);
    const auto actor_loss =
        torch::mean(entropy_parameter.alpha().detach() * curr_log_prob.sum(-1) - q_value.detach());

    actor_optimizer->zero_grad();
    actor_loss.backward();
    actor_optimizer->step();

    // entropy
    const auto entropy_loss = -torch::mean(
        entropy_parameter.log_alpha() * (curr_log_prob.sum(-1).detach() + target_entropy));

    entropy_optimizer.zero_grad();
    entropy_loss.backward();
    entropy_optimizer.step();

    // target value soft update
    for (auto n_p: critic_1->named_parameters()) {
        const auto &name = n_p.key();
        const auto &param = n_p.value();

        auto target_param = target_critic_1->named_parameters()[name];
        target_param.data().copy_(tau * param.data() + (1.f - tau) * target_param.data());
    }

    for (auto n_p: critic_2->named_parameters()) {
        const auto &name = n_p.key();
        const auto &param = n_p.value();

        auto target_param = target_critic_2->named_parameters()[name];
        target_param.data().copy_(tau * param.data() + (1.f - tau) * target_param.data());
    }

    // metrics
    actor_loss_meter.add(actor_loss.cpu().item().toFloat());
    critic_1_loss_meter.add(critic_1_loss.cpu().item().toFloat());
    critic_2_loss_meter.add(critic_2_loss.cpu().item().toFloat());
    entropy_loss_meter.add(entropy_loss.cpu().item().toFloat());

    curr_train_step++;
}

void SoftActorCritic::done(torch::Tensor state, float reward) {
    const auto [mu, sigma] = actor->forward(state);
    const auto action = truncated_normal_sample(mu, sigma, -1.f, 1.f);

    const auto [target_q_value_1] = target_critic_1->forward(state, action.detach());
    const auto [target_q_value_2] = target_critic_2->forward(state, action.detach());

    episodes_buffer.back().rewards_buffer.push_back(reward);
    episodes_buffer.back().mu_buffer.push_back(mu);
    episodes_buffer.back().sigma_buffer.push_back(sigma);
    episodes_buffer.back().target_q_value_1_buffer.push_back(target_q_value_1);
    episodes_buffer.back().target_q_value_2_buffer.push_back(target_q_value_2);
    episodes_buffer.back().actions_buffer.push_back(action);
    episodes_buffer.back().done_buffer.push_back(1.f);

    episodes_buffer.back().done_buffer.erase(episodes_buffer.back().done_buffer.begin());
    episodes_buffer.back().rewards_buffer.erase(episodes_buffer.back().rewards_buffer.begin());

    if (actor->is_training() && static_cast<int>(episodes_buffer.size()) == batch_size) {
        int episode_max_step = 0;

        std::vector<torch::Tensor> actions_per_episode, mus_per_episode, sigmas_per_episode,
            q_values_1_per_episode, q_values_2_per_episode, target_q_values_1_per_episode,
            target_q_values_2_per_episode, rewards_per_episode, done_per_episode;

        for (const auto &t: episodes_buffer)
            episode_max_step =
                std::max(static_cast<int>(t.rewards_buffer.size()), episode_max_step);

        for (const auto
                 &[mu_buffer, sigma_buffer, q_value_1_buffer, q_value_2_buffer,
                   target_q_value_1_buffer, target_q_value_2_buffer, rewards_buffer, actions_buffer,
                   done_buffer]: episodes_buffer) {
            int pad = episode_max_step - static_cast<int>(rewards_buffer.size());

            actions_per_episode.push_back(torch::pad(torch::stack(actions_buffer), {0, 0, 0, pad}));

            q_values_1_per_episode.push_back(torch::pad(torch::cat(q_value_1_buffer), {0, pad}));
            q_values_2_per_episode.push_back(torch::pad(torch::cat(q_value_2_buffer), {0, pad}));

            target_q_values_1_per_episode.push_back(
                torch::pad(torch::cat(target_q_value_1_buffer), {0, pad}));
            target_q_values_2_per_episode.push_back(
                torch::pad(torch::cat(target_q_value_2_buffer), {0, pad}));

            mus_per_episode.push_back(torch::pad(torch::stack(mu_buffer), {0, 0, 0, pad}));
            sigmas_per_episode.push_back(
                torch::pad(torch::stack(sigma_buffer), {0, 0, 0, pad}, "constant", 1.f));

            rewards_per_episode.push_back(torch::pad(
                torch::tensor(rewards_buffer, at::TensorOptions().device(curr_device)), {0, pad}));

            done_per_episode.push_back(torch::pad(
                torch::tensor(done_buffer, at::TensorOptions().device(curr_device)), {0, pad},
                "constant", 1.f));
        }

        train(
            torch::stack(actions_per_episode), torch::stack(q_values_1_per_episode),
            torch::stack(q_values_2_per_episode), torch::stack(target_q_values_1_per_episode),
            torch::stack(target_q_values_2_per_episode), torch::stack(mus_per_episode),
            torch::stack(sigmas_per_episode), torch::stack(rewards_per_episode),
            torch::stack(done_per_episode));

        episodes_buffer.clear();
    } else if (!actor->is_training()) {
        episodes_buffer.clear();
    }

    episodes_buffer.push_back({});

    episode_steps_meter.add(static_cast<float>(curr_episode_step));
    curr_episode_step = 0;
}

void SoftActorCritic::save(const std::string &output_folder_path) {
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

    // target_critic_1
    const auto target_critic_1_model_file = path / "target_critic_1.th";
    torch::serialize::OutputArchive target_critic_1_model_archive;
    target_critic_1->save(target_critic_1_model_archive);
    target_critic_1_model_archive.save_to(target_critic_1_model_file);

    // target_critic_2
    const auto target_critic_2_model_file = path / "target_critic_2.th";
    torch::serialize::OutputArchive target_critic_2_model_archive;
    target_critic_2->save(target_critic_2_model_archive);
    target_critic_2_model_archive.save_to(target_critic_2_model_file);

    // Critic 1
    const auto critic_1_model_file = path / "critic_1.th";
    const auto critic_1_optimizer_file = path / "critic_1_optimizer.th";

    torch::serialize::OutputArchive critic_1_model_archive;
    torch::serialize::OutputArchive critic_1_optimizer_archive;

    critic_1->save(critic_1_model_archive);
    critic_1_optimizer->save(critic_1_optimizer_archive);

    critic_1_model_archive.save_to(critic_1_model_file);
    critic_1_optimizer_archive.save_to(critic_1_optimizer_file);

    // Critic 2
    const auto critic_2_model_file = path / "critic_2.th";
    const auto critic_2_optimizer_file = path / "critic_2_optimizer.th";

    torch::serialize::OutputArchive critic_2_model_archive;
    torch::serialize::OutputArchive critic_2_optimizer_archive;

    critic_2->save(critic_2_model_archive);
    critic_2_optimizer->save(critic_2_optimizer_archive);

    critic_2_model_archive.save_to(critic_2_model_file);
    critic_2_optimizer_archive.save_to(critic_2_optimizer_file);

    // entropy
    const auto entropy_model_file = path / "entropy.th";
    const auto entropy_optimizer_file = path / "entropy_optimizer.th";

    torch::serialize::OutputArchive entropy_model_archive;
    torch::serialize::OutputArchive entropy_optimizer_archive;

    entropy_parameter.save(entropy_model_archive);
    entropy_optimizer.save(entropy_optimizer_archive);

    entropy_model_archive.save_to(entropy_model_file);
    entropy_optimizer_archive.save_to(entropy_optimizer_file);
}

void SoftActorCritic::load(const std::string &input_folder_path) {
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

    // target_critic_1_network
    const auto target_critic_1_model_file = path / "target_critic_1.th";
    torch::serialize::InputArchive target_critic_1_model_archive;
    target_critic_1_model_archive.load_from(target_critic_1_model_file);
    target_critic_1->load(target_critic_1_model_archive);

    // target_critic_2_network
    const auto target_critic_2_model_file = path / "target_critic_2.th";
    torch::serialize::InputArchive target_critic_2_model_archive;
    target_critic_2_model_archive.load_from(target_critic_2_model_file);
    target_critic_2->load(target_critic_2_model_archive);

    // critic_1
    const auto critic_1_model_file = path / "critic_1.th";
    const auto critic_1_optimizer_file = path / "critic_1_optimizer.th";

    torch::serialize::InputArchive critic_1_model_archive;
    torch::serialize::InputArchive critic_1_optimizer_archive;

    critic_1_model_archive.load_from(critic_1_model_file);
    critic_1_optimizer_archive.load_from(critic_1_optimizer_file);

    critic_1->load(critic_1_model_archive);
    critic_1_optimizer->load(critic_1_optimizer_archive);

    // critic_2
    const auto critic_2_model_file = path / "critic_2.th";
    const auto critic_2_optimizer_file = path / "critic_2_optimizer.th";

    torch::serialize::InputArchive critic_2_model_archive;
    torch::serialize::InputArchive critic_2_optimizer_archive;

    critic_2_model_archive.load_from(critic_2_model_file);
    critic_2_optimizer_archive.load_from(critic_2_optimizer_file);

    critic_2->load(critic_2_model_archive);
    critic_2_optimizer->load(critic_2_optimizer_archive);

    // entropy
    const auto entropy_model_file = path / "entropy.th";
    const auto entropy_optimizer_file = path / "entropy_optimizer.th";

    torch::serialize::InputArchive entropy_model_archive;
    torch::serialize::InputArchive entropy_optimizer_archive;

    entropy_model_archive.load_from(entropy_model_file);
    entropy_optimizer_archive.load_from(entropy_optimizer_file);

    entropy_parameter.load(entropy_model_archive);
    entropy_optimizer.load(entropy_optimizer_archive);
}

std::vector<LossMeter> SoftActorCritic::get_metrics() {
    return {
        actor_loss_meter, critic_1_loss_meter, critic_2_loss_meter, entropy_loss_meter,
        episode_steps_meter};
}

void SoftActorCritic::to(torch::DeviceType device) {
    curr_device = device;
    actor->to(device);
    critic_1->to(device);
    critic_2->to(device);
    target_critic_1->to(device);
    target_critic_2->to(device);
    entropy_parameter.to(device);
}

void SoftActorCritic::set_eval(bool eval) {
    if (eval) {
        actor->eval();
        critic_1->eval();
        critic_2->eval();
        target_critic_1->eval();
        target_critic_2->eval();
        entropy_parameter.eval();
    } else {
        actor->train();
        critic_1->train();
        critic_2->train();
        target_critic_1->train();
        target_critic_2->train();
        entropy_parameter.train();
    }
}

int SoftActorCritic::count_parameters() {
    int count = 0;
    for (const auto &p: actor->parameters()) {
        auto sizes = p.sizes();
        count += std::reduce(sizes.begin(), sizes.end(), 1, std::multiplies<>());
    }
    for (const auto &p: target_critic_1->parameters()) {
        auto sizes = p.sizes();
        count += std::reduce(sizes.begin(), sizes.end(), 1, std::multiplies<>());
    }
    for (const auto &p: target_critic_2->parameters()) {
        auto sizes = p.sizes();
        count += std::reduce(sizes.begin(), sizes.end(), 1, std::multiplies<>());
    }
    for (const auto &p: critic_1->parameters()) {
        auto sizes = p.sizes();
        count += std::reduce(sizes.begin(), sizes.end(), 1, std::multiplies<>());
    }
    for (const auto &p: critic_2->parameters()) {
        auto sizes = p.sizes();
        count += std::reduce(sizes.begin(), sizes.end(), 1, std::multiplies<>());
    }
    return count;
}
