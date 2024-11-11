//
// Created by samuel on 19/12/22.
//

#include <algorithm>
#include <filesystem>
#include <numeric>

#include <torch/torch.h>

#include <evo_motion_networks/agents/actor_critic.h>
#include <evo_motion_networks/functions.h>

ActorCriticAgent::ActorCriticAgent(
    const int seed, const std::vector<int64_t> &state_space,
    const std::vector<int64_t> &action_space, int hidden_size, const int batch_size, float actor_lr, float critic_lr,
    const float gamma, const float entropy_start_factor, float entropy_end_factor,
    long entropy_steps, int replay_buffer_size)
    : actor(std::make_shared<ActorModule>(state_space, action_space, hidden_size)),
      actor_optimizer(std::make_shared<torch::optim::Adam>(actor->parameters(), actor_lr)),
      critic(std::make_shared<CriticModule>(state_space, hidden_size)),
      critic_optimizer(std::make_shared<torch::optim::Adam>(critic->parameters(), critic_lr)),
      gamma(gamma), entropy_start_factor(entropy_start_factor),
      entropy_end_factor(entropy_end_factor), entropy_steps(entropy_steps),
      curr_device(torch::kCPU), batch_size(batch_size), replay_buffer(replay_buffer_size, seed),
      policy_loss_meter("policy", 64), entropy_meter("entropy", 64),
      critic_loss_meter("critic", 64), episode_steps_meter("steps", 64), curr_episode_step(0),
      curr_train_step(0L), global_curr_step(0L) {
    at::manual_seed(seed);
}

torch::Tensor ActorCriticAgent::act(const torch::Tensor state, const float reward) {
    const auto [mu, sigma] = actor->forward(state);
    const auto [value] = critic->forward(state);

    auto action = truncated_normal_sample(mu, sigma, -1.f, 1.f);

    if (!replay_buffer.empty()) { replay_buffer.update_last(reward, state, false); }
    replay_buffer.add({state, action, 0.f, false, state});

    curr_episode_step++;
    global_curr_step++;

    check_train();

    return action;
}

void ActorCriticAgent::check_train() {
    if (global_curr_step % batch_size == 0) {
        std::vector<step_replay_buffer> tmp_replay_buffer = replay_buffer.sample(batch_size);

        std::vector<torch::Tensor> vec_states, vec_actions, vec_rewards, vec_done, vec_next_state;

        for (int i = 0; i < batch_size; i++) {
            const auto &rp = tmp_replay_buffer[i];

            vec_states.push_back(rp.state);
            vec_actions.push_back(rp.action);
            vec_rewards.push_back(
                torch::tensor(rp.reward, at::TensorOptions().device(curr_device)));
            vec_done.push_back(
                torch::tensor(rp.done ? 1.f : 0.f, at::TensorOptions().device(curr_device)));
            vec_next_state.push_back(rp.next_state);
        }

        train(
            torch::stack(vec_states), torch::stack(vec_actions), torch::stack(vec_rewards).unsqueeze(1),
            torch::stack(vec_done).unsqueeze(1), torch::stack(vec_next_state));
    }
}

void ActorCriticAgent::train(
    const torch::Tensor &batched_states, const torch::Tensor &batched_actions,
    const torch::Tensor &batched_rewards, const torch::Tensor &batched_done,
    const torch::Tensor &batched_next_state) {

    const auto [next_value] = critic->forward(batched_next_state);
    const auto [value] = critic->forward(batched_states);

    const auto target = batched_rewards + (1.f - batched_done) * gamma * next_value;
    const auto advantage = target - value;

    const auto critic_loss = torch::mse_loss(value, target, at::Reduction::Mean);

    critic_optimizer->zero_grad();
    critic_loss.backward();
    critic_optimizer->step();

    const auto [mu, sigma] = actor->forward(batched_states);
    const auto log_prob = truncated_normal_pdf(batched_actions.detach(), mu, sigma, -1.f, 1.f);
    const auto policy_entropy =
        truncated_normal_entropy(mu, sigma, -1.f, 1.f)
        * exponential_decrease(
            curr_train_step, entropy_steps, entropy_start_factor, entropy_end_factor);
    const auto policy_loss = log_prob * advantage.detach().unsqueeze(-1);

    const auto actor_loss = -torch::mean(policy_loss + policy_entropy);

    actor_optimizer->zero_grad();
    actor_loss.backward();
    actor_optimizer->step();

    policy_loss_meter.add(-policy_loss.sum(-1).mean().cpu().item().toFloat());
    entropy_meter.add(-policy_entropy.sum(-1).mean().cpu().item().toFloat());
    critic_loss_meter.add(critic_loss.cpu().item().toFloat());

    curr_train_step++;
}

void ActorCriticAgent::done(torch::Tensor state, const float reward) {
    replay_buffer.update_last(reward, state, true);

    episode_steps_meter.add(static_cast<float>(curr_episode_step));
    curr_episode_step = 0;
}

void ActorCriticAgent::save(const std::string &output_folder_path) {
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

void ActorCriticAgent::load(const std::string &input_folder_path) {
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

std::vector<LossMeter> ActorCriticAgent::get_metrics() {
    return {policy_loss_meter, entropy_meter, critic_loss_meter, episode_steps_meter};
}

void ActorCriticAgent::to(const torch::DeviceType device) {
    curr_device = device;
    actor->to(curr_device);
    critic->to(curr_device);
}

void ActorCriticAgent::set_eval(const bool eval) {
    if (eval) {
        actor->eval();
        critic->eval();
    } else {
        actor->train();
        critic->train();
    }
}

int ActorCriticAgent::count_parameters() {
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