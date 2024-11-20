//
// Created by samuel on 13/11/24.
//

#include <evo_motion_networks/agents/ppo.h>
#include <evo_motion_networks/functions.h>
#include <evo_motion_networks/saver.h>

ProximalPolicyOptimizationAgent::ProximalPolicyOptimizationAgent(
    const int seed, const std::vector<int64_t> &state_space,
    const std::vector<int64_t> &action_space, int hidden_size, const float gamma, const float lam,
    const float epsilon, const float entropy_factor, const float critic_loss_factor,
    const int epoch,
    const int batch_size, float learning_rate, const int replay_buffer_size, const int train_every,
    const float grad_norm_clip)
    : actor(std::make_shared<ActorModule>(state_space, action_space, hidden_size)),
      actor_optimizer(std::make_shared<torch::optim::Adam>(actor->parameters(), learning_rate)),
      critic(std::make_shared<CriticModule>(state_space, hidden_size)),
      critic_optimizer(std::make_shared<torch::optim::Adam>(critic->parameters(), learning_rate)),
      gamma(gamma), lambda(lam), epsilon(epsilon), epoch(epoch),
      entropy_factor(entropy_factor), critic_loss_factor(critic_loss_factor),
      grad_norm_clip(grad_norm_clip),
      curr_train_step(0L), curr_episode_step(0L), global_curr_step(0L), batch_size(batch_size),
      replay_buffer(replay_buffer_size, seed), train_every(train_every),
      actor_loss_meter("actor_loss", 64), critic_loss_meter("critic_loss", 64),
      episode_steps_meter("steps", 64), curr_device(torch::kCPU) {

    at::manual_seed(seed);
}

torch::Tensor ProximalPolicyOptimizationAgent::act(const torch::Tensor state, const float reward) {
    set_eval(true);

    const auto [mu, sigma] = actor->forward(state);
    const auto action = truncated_normal_sample(mu, sigma, -1.f, 1.f);

    if (!replay_buffer.empty()) { replay_buffer.update_last(reward, state, false); }
    replay_buffer.add({state, action.detach(), 0.f, false, state});

    curr_episode_step++;
    global_curr_step++;

    check_train();

    return action;
}

void ProximalPolicyOptimizationAgent::done(const torch::Tensor state, const float reward) {
    replay_buffer.update_last(reward, state, true);

    episode_steps_meter.add(static_cast<float>(curr_episode_step));
    curr_episode_step = 0;
}

void ProximalPolicyOptimizationAgent::check_train() {
    if (global_curr_step % train_every == train_every - 1) {
        std::vector<episode_step> tmp_replay_buffer = replay_buffer.sample(batch_size);

        std::vector<torch::Tensor> vec_states, vec_actions, vec_rewards, vec_done, vec_next_state;

        for (const auto &[state, action, reward, done, next_state]: tmp_replay_buffer) {
            vec_states.push_back(state);
            vec_actions.push_back(action);
            vec_rewards.push_back(
                torch::tensor({reward}, at::TensorOptions().device(curr_device)));
            vec_done.push_back(
                torch::tensor({done ? 1.f : 0.f}, at::TensorOptions().device(curr_device)));
            vec_next_state.push_back(next_state);
        }

        train(
            torch::stack(vec_states), torch::stack(vec_actions),
            torch::stack(vec_rewards), torch::stack(vec_done),
            torch::stack(vec_next_state));
    }
}

void ProximalPolicyOptimizationAgent::train(
    const torch::Tensor &batched_states, const torch::Tensor &batched_actions,
    const torch::Tensor &batched_rewards, const torch::Tensor &batched_done,
    const torch::Tensor &batched_next_state) {

    //torch::autograd::DetectAnomalyGuard guard;

    set_eval(false);

    const auto [curr_values] = critic->forward(batched_states);
    const auto [next_values] = critic->forward(batched_next_state);

    //const auto norm_rewards = (batched_rewards - batched_rewards.mean()) / (batched_rewards.std() + 1e-8);

    const auto advantages =
        batched_rewards + (1.f - batched_done) * gamma * next_values - curr_values;
    const auto returns = batched_rewards + (1.f - batched_done) * gamma * next_values;

    const auto norm_advantages = (advantages - advantages.mean()) / (advantages.std() + 1e-8);
    const auto norm_returns = (returns - returns.mean()) / (returns.std() + 1e-8);

    constexpr float min_proba = 1e-8;
    constexpr float max_proba = 1e8;

    const auto [old_mu, old_sigma] = actor->forward(batched_states);
    const auto old_log_prob = torch::clamp(
        truncated_normal_log_pdf(batched_actions, old_mu, old_sigma, -1.f, 1.f), std::log(min_proba),
        std::log(max_proba));

    for (int i = 0; i < epoch; i++) {
        const auto [mu, sigma] = actor->forward(batched_states);
        const auto log_prob =
            torch::clamp(
                truncated_normal_log_pdf(batched_actions, mu, sigma, -1.f, 1.f), std::log(min_proba),
                std::log(max_proba));
        const auto entropy = truncated_normal_entropy(mu, sigma, -1.f, 1.f);

        const auto [value] = critic->forward(batched_states);

        const auto ratios = log_prob - old_log_prob.detach();

        const auto surrogate_1 = ratios * norm_advantages.detach();
        const auto surrogate_2 =
            torch::clamp(ratios, std::log(1.f - epsilon), std::log(1.f + epsilon)) * norm_advantages.detach();

        // actor
        const auto actor_loss =
            -torch::mean(torch::min(surrogate_1, surrogate_2) + entropy_factor * entropy);

        actor_optimizer->zero_grad();
        actor_loss.backward();
        torch::nn::utils::clip_grad_norm_(actor->parameters(), grad_norm_clip);
        actor_optimizer->step();

        // critic
        const auto critic_loss =
            critic_loss_factor
            * torch::mse_loss(value, norm_returns.detach(), torch::Reduction::Mean);

        critic_optimizer->zero_grad();
        critic_loss.backward();
        torch::nn::utils::clip_grad_norm_(critic->parameters(), grad_norm_clip);
        critic_optimizer->step();

        actor_loss_meter.add(actor_loss.item().toFloat());
        critic_loss_meter.add(critic_loss.item().toFloat());
    }

    curr_train_step++;
}

void ProximalPolicyOptimizationAgent::save(const std::string &output_folder_path) {
    save_torch(output_folder_path, actor, "actor.th");
    save_torch(output_folder_path, actor_optimizer, "actor_optimizer.th");
    save_torch(output_folder_path, critic, "critic.th");
    save_torch(output_folder_path, critic_optimizer, "critic_optimizer");
}

void ProximalPolicyOptimizationAgent::load(const std::string &input_folder_path) {
    load_torch(input_folder_path, actor, "actor.th");
    load_torch(input_folder_path, actor_optimizer, "actor_optimizer.th");
    load_torch(input_folder_path, critic, "critic.th");
    load_torch(input_folder_path, critic_optimizer, "critic_optimizer");
}

std::vector<LossMeter> ProximalPolicyOptimizationAgent::get_metrics() {
    return {actor_loss_meter, critic_loss_meter, episode_steps_meter};
}

void ProximalPolicyOptimizationAgent::to(torch::DeviceType device) {
    curr_device = device;
    actor->to(device);
    critic->to(device);
}

void ProximalPolicyOptimizationAgent::set_eval(bool eval) {
    if (eval) {
        actor->eval();
        critic->eval();
    } else {
        actor->train();
        critic->train();
    }
}

int ProximalPolicyOptimizationAgent::count_parameters() {
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
