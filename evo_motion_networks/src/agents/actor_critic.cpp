//
// Created by samuel on 19/12/22.
//

#include <algorithm>
#include <filesystem>
#include <numeric>

#include <torch/torch.h>

#include <evo_motion_networks/agents/actor_critic.h>
#include <evo_motion_networks/functions.h>
#include <evo_motion_networks/saver.h>

ActorCriticAgent::ActorCriticAgent(
    const int seed, const std::vector<int64_t> &state_space,
    const std::vector<int64_t> &action_space, int hidden_size, const int batch_size,
    float learning_rate, const float gamma, const float entropy_start_factor,
    const float entropy_end_factor, const long entropy_steps, const int replay_buffer_size,
    const int train_every)
    : actor(std::make_shared<ActorModule>(state_space, action_space, hidden_size)),
      actor_optimizer(std::make_shared<torch::optim::Adam>(actor->parameters(), learning_rate)),
      critic(std::make_shared<CriticModule>(state_space, hidden_size)),
      critic_optimizer(std::make_shared<torch::optim::Adam>(critic->parameters(), learning_rate)),
      gamma(gamma), entropy_start_factor(entropy_start_factor),
      entropy_end_factor(entropy_end_factor), entropy_steps(entropy_steps),
      curr_device(torch::kCPU), batch_size(batch_size), replay_buffer(replay_buffer_size, seed),
      policy_loss_meter("policy", 64), entropy_meter("entropy", 64),
      critic_loss_meter("critic", 64), episode_steps_meter("steps", 64), curr_episode_step(0),
      curr_train_step(0L), global_curr_step(0L), train_every(train_every) {
    at::manual_seed(seed);

    set_eval(true);
}

torch::Tensor ActorCriticAgent::act(const torch::Tensor state, const float reward) {
    const auto [mu, sigma] = actor->forward(state);

    auto action = truncated_normal_sample(mu, sigma, -1.f, 1.f);

    if (!replay_buffer.empty()) { replay_buffer.update_last(reward, state, false); }
    replay_buffer.add({state, action, 0.f, false, state});

    check_train();

    curr_episode_step++;
    global_curr_step++;

    return action;
}

void ActorCriticAgent::check_train() {
    if (global_curr_step % train_every == train_every - 1 && replay_buffer.has_enough(batch_size)) {
        std::vector<episode_step> tmp_replay_buffer = replay_buffer.sample(batch_size);

        std::vector<torch::Tensor> vec_states, vec_actions, vec_rewards, vec_done, vec_next_state;

        for (const auto &[state, action, reward, done, next_state]: tmp_replay_buffer) {
            vec_states.push_back(state);
            vec_actions.push_back(action);
            vec_rewards.push_back(torch::tensor({reward}, at::TensorOptions().device(curr_device)));
            vec_done.push_back(
                torch::tensor({done ? 1.f : 0.f}, at::TensorOptions().device(curr_device)));
            vec_next_state.push_back(next_state);
        }

        train(
            torch::stack(vec_states), torch::stack(vec_actions), torch::stack(vec_rewards),
            torch::stack(vec_done), torch::stack(vec_next_state));
    }
}

void ActorCriticAgent::train(
    const torch::Tensor &batched_states, const torch::Tensor &batched_actions,
    const torch::Tensor &batched_rewards, const torch::Tensor &batched_done,
    const torch::Tensor &batched_next_state) {

    set_eval(false);

    const auto [next_value] = critic->forward(batched_next_state);
    const auto [value] = critic->forward(batched_states);

    const auto norm_reward =
        (batched_rewards - batched_rewards.mean()) / (batched_rewards.std() + 1e-8);
    const auto target = norm_reward + (1.f - batched_done) * gamma * next_value;

    const auto critic_loss = torch::mse_loss(value, target.detach(), at::Reduction::Mean);

    critic_optimizer->zero_grad();
    critic_loss.backward();
    critic_optimizer->step();

    const auto [mu, sigma] = actor->forward(batched_states);
    const auto log_prob = truncated_normal_log_pdf(batched_actions.detach(), mu, sigma, -1.f, 1.f);
    const auto policy_entropy =
        truncated_normal_entropy(mu, sigma, -1.f, 1.f)
        * exponential_decrease(
            curr_train_step, entropy_steps, entropy_start_factor, entropy_end_factor);
    const auto policy_loss = log_prob * (target - value).detach().unsqueeze(-1);

    const auto actor_loss = -torch::mean(torch::sum(policy_loss + policy_entropy, -1));

    actor_optimizer->zero_grad();
    actor_loss.backward();
    actor_optimizer->step();

    policy_loss_meter.add(-policy_loss.sum(-1).mean().cpu().item().toFloat());
    entropy_meter.add(-policy_entropy.sum(-1).mean().cpu().item().toFloat());
    critic_loss_meter.add(critic_loss.cpu().item().toFloat());

    set_eval(true);

    curr_train_step++;
}

void ActorCriticAgent::done(const torch::Tensor state, const float reward) {
    replay_buffer.update_last(reward, state, true);

    episode_steps_meter.add(static_cast<float>(curr_episode_step));
    curr_episode_step = 0;
}

void ActorCriticAgent::save(const std::string &output_folder_path) {
    const std::filesystem::path path(output_folder_path);

    // actor
    save_torch(output_folder_path, actor, "actor.th");
    save_torch(output_folder_path, actor_optimizer, "actor.th");

    // critic
    save_torch(output_folder_path, critic, "critic.th");
    save_torch(output_folder_path, critic_optimizer, "critic_optimizer.th");
}

void ActorCriticAgent::load(const std::string &input_folder_path) {
    const std::filesystem::path path(input_folder_path);

    // actor
    load_torch(input_folder_path, actor, "actor.th");
    load_torch(input_folder_path, actor_optimizer, "actor_optimizer.th");

    // critic
    load_torch(input_folder_path, critic, "critic.th");
    load_torch(input_folder_path, critic_optimizer, "critic_optimizer.th");
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
    actor->train(!eval);
    critic->train(!eval);
}

int ActorCriticAgent::count_parameters() {
    return count_module_parameters(actor) + count_module_parameters(critic);
}