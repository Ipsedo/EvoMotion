//
// Created by samuel on 13/11/24.
//

#include <evo_motion_networks/agents/ppo_gae.h>
#include <evo_motion_networks/functions.h>
#include <evo_motion_networks/saver.h>

PpoGaeAgent::PpoGaeAgent(
    const int seed, const std::vector<int64_t> &state_space,
    const std::vector<int64_t> &action_space, int hidden_size, const float gamma, const float lam,
    const float epsilon, const float entropy_factor, const float critic_loss_factor,
    const int epoch, const int batch_size, float learning_rate, float clip_grad_norm)
    : actor(std::make_shared<ActorModule>(state_space, action_space, hidden_size)),
      actor_optimizer(std::make_shared<torch::optim::Adam>(actor->parameters(), learning_rate)),
      critic(std::make_shared<CriticModule>(state_space, hidden_size)),
      critic_optimizer(std::make_shared<torch::optim::Adam>(critic->parameters(), learning_rate)),
      gamma(gamma), lambda(lam), epsilon(epsilon), epoch(epoch), entropy_factor(entropy_factor),
      critic_loss_factor(critic_loss_factor), clip_grad_norm(clip_grad_norm), curr_train_step(0L),
      curr_episode_step(0L), global_curr_step(0L), batch_size(batch_size),
      replay_buffer(batch_size, seed), train_every(batch_size), actor_loss_meter("actor_loss", 64),
      critic_loss_meter("critic_loss", 64), episode_steps_meter("steps", 64),
      curr_device(torch::kCPU) {

    at::manual_seed(seed);

    actor->to(torch::kFloat64);
    critic->to(torch::kFloat64);
}

torch::Tensor PpoGaeAgent::act(const torch::Tensor state, const float reward) {
    set_eval(true);

    const auto state_double = state.to(torch::kFloat64);

    const auto [mu, sigma] = actor->forward(state_double);
    const auto action = truncated_normal_sample(mu, sigma, -1.f, 1.f);

    if (replay_buffer.empty()) replay_buffer.new_trajectory();
    if (!replay_buffer.trajectory_empty()) replay_buffer.update_last(reward, false, state_double);
    replay_buffer.add({state_double, action.detach(), 0.f, false, state_double});

    curr_episode_step++;

    return action;
}

void PpoGaeAgent::done(const torch::Tensor state, const float reward) {
    const auto state_double = state.to(torch::kFloat64);
    replay_buffer.update_last(reward, true, state_double);

    check_train();

    replay_buffer.new_trajectory();
    global_curr_step++;

    episode_steps_meter.add(static_cast<float>(curr_episode_step));
    curr_episode_step = 0L;
}

void PpoGaeAgent::check_train() {
    if ((global_curr_step % train_every == train_every - 1)
        && replay_buffer.enough_trajectory(batch_size)) {

        const auto episodes = replay_buffer.sample(batch_size);

        std::vector<torch::Tensor> batch_vec_states, batch_vec_actions, batch_vec_rewards,
            batch_vec_done, batch_vec_next_states;

        int max_steps = 0;
        for (const auto &[trajectory]: episodes)
            max_steps = std::max(max_steps, static_cast<int>(trajectory.size()));

        for (const auto &[trajectory]: episodes) {
            std::vector<torch::Tensor> vec_states, vec_actions, vec_rewards, vec_done,
                vec_next_states;

            for (const auto &[state, action, reward, done, next_state]: trajectory) {
                vec_states.push_back(state);
                vec_actions.push_back(action);
                vec_rewards.push_back(
                    torch::tensor({reward}, torch::TensorOptions().device(curr_device)));
                vec_done.push_back(
                    torch::tensor({done ? 1.f : 0.f}, torch::TensorOptions().device(curr_device)));
                vec_next_states.push_back(next_state);
            }

            const int pad = max_steps - static_cast<int>(trajectory.size());

            batch_vec_states.push_back(torch::pad(torch::stack(vec_states), {0, 0, 0, pad}));
            batch_vec_actions.push_back(torch::pad(torch::stack(vec_actions), {0, 0, 0, pad}));
            batch_vec_rewards.push_back(torch::pad(torch::stack(vec_rewards), {0, 0, 0, pad}));
            batch_vec_done.push_back(
                torch::pad(torch::stack(vec_done), {0, 0, 0, pad}, "constant", 1.f));
            batch_vec_next_states.push_back(
                torch::pad(torch::stack(vec_next_states), {0, 0, 0, pad}));
        }

        train(
            torch::stack(batch_vec_states), torch::stack(batch_vec_actions),
            torch::stack(batch_vec_rewards), torch::stack(batch_vec_done),
            torch::stack(batch_vec_next_states));
    }
}

void PpoGaeAgent::train(
    const torch::Tensor &batched_states, const torch::Tensor &batched_actions,
    const torch::Tensor &batched_rewards, const torch::Tensor &batched_done,
    const torch::Tensor &batched_next_state) {

    torch::autograd::DetectAnomalyGuard guard;

    set_eval(false);

    const auto [curr_values] = critic->forward(batched_states);
    const auto [next_values] = critic->forward(batched_next_state);

    const auto mask = torch::eq(
        torch::cat(
            {torch::ones({batched_done.size(0), 1, 1}, at::TensorOptions().device(curr_device)),
             torch::slice(1.f - batched_done, 1, 0, batched_done.size(1) - 1)},
            1),
        1.f);

    /*const auto target = batched_rewards + (1.f - batched_done) * gamma * next_values;

    const auto gae_factor =
        torch::pow(
            gamma * lambda,
            torch::arange(batched_rewards.size(1), torch::TensorOptions().device(curr_device)))
            .unsqueeze(0)
            .unsqueeze(-1);

    auto advantages =
        (mask * (target - curr_values) * gae_factor).flip({1}).cumsum(1).flip({1}) / gae_factor;
    advantages = (advantages - torch::masked_select(advantages, mask).mean())
                 / (torch::masked_select(advantages, mask).std() + 1e-8f);
    const auto returns = advantages + curr_values;*/

    const auto deltas = batched_rewards + (1.f - batched_done) * gamma * next_values - curr_values;

    auto gae_step =
        torch::zeros({batched_states.size(0), 1}, at::TensorOptions().device(curr_device));
    std::vector<torch::Tensor> advantages_vec;

    for (int t = static_cast<int>(batched_rewards.size(1)) - 1; t >= 0; t--) {
        gae_step = torch::select(deltas, 1, t) * torch::select(mask, 1, t)
                   + gamma * lambda * (1.f - torch::select(batched_done, 1, t)) * gae_step;
        advantages_vec.push_back(gae_step);
    }

    auto advantages = torch::stack(advantages_vec, 1).flip({1});
    advantages = (advantages - torch::masked_select(advantages, mask).mean())
                 / (torch::masked_select(advantages, mask).std() + 1e-8f);
    const auto returns = advantages + curr_values;

    const auto [old_mu, old_sigma] = actor->forward(batched_states);
    const auto old_log_prob =
        truncated_normal_log_pdf(batched_actions, old_mu, old_sigma, -1.f, 1.f);

    for (int i = 0; i < epoch; i++) {
        const auto [mu, sigma] = actor->forward(batched_states);
        const auto log_prob = truncated_normal_log_pdf(batched_actions, mu, sigma, -1.f, 1.f);
        const auto entropy = truncated_normal_entropy(mu, sigma, -1.f, 1.f);

        const auto [value] = critic->forward(batched_states);

        // actor
        const auto ratios = torch::exp(log_prob - old_log_prob.detach());

        const auto surrogate_1 = ratios * advantages.detach();
        const auto surrogate_2 =
            torch::clamp(ratios, 1.f - epsilon, 1.f + epsilon) * advantages.detach();

        const auto actor_loss = -torch::mean(torch::masked_select(
            torch::sum(torch::min(surrogate_1, surrogate_2) + entropy_factor * entropy, -1, true),
            mask));

        actor_optimizer->zero_grad();
        actor_loss.backward();
        torch::nn::utils::clip_grad_norm_(actor->parameters(), clip_grad_norm);
        actor_optimizer->step();

        // critic
        const auto critic_loss =
            critic_loss_factor
            * torch::mean(torch::masked_select(
                torch::mse_loss(value, returns.detach(), torch::Reduction::None), mask));

        critic_optimizer->zero_grad();
        critic_loss.backward();
        torch::nn::utils::clip_grad_norm_(critic->parameters(), clip_grad_norm);
        critic_optimizer->step();

        actor_loss_meter.add(actor_loss.item().toFloat());
        critic_loss_meter.add(critic_loss.item().toFloat());
    }

    curr_train_step++;
}

void PpoGaeAgent::save(const std::string &output_folder_path) {
    save_torch(output_folder_path, actor, "actor.th");
    save_torch(output_folder_path, actor_optimizer, "actor_optimizer.th");
    save_torch(output_folder_path, critic, "critic.th");
    save_torch(output_folder_path, critic_optimizer, "critic_optimizer");
}

void PpoGaeAgent::load(const std::string &input_folder_path) {
    load_torch(input_folder_path, actor, "actor.th");
    load_torch(input_folder_path, actor_optimizer, "actor_optimizer.th");
    load_torch(input_folder_path, critic, "critic.th");
    load_torch(input_folder_path, critic_optimizer, "critic_optimizer");
}

std::vector<LossMeter> PpoGaeAgent::get_metrics() {
    return {actor_loss_meter, critic_loss_meter, episode_steps_meter};
}

void PpoGaeAgent::to(torch::DeviceType device) {
    curr_device = device;
    actor->to(device);
    critic->to(device);
}

void PpoGaeAgent::set_eval(bool eval) {
    if (eval) {
        actor->eval();
        critic->eval();
    } else {
        actor->train();
        critic->train();
    }
}

int PpoGaeAgent::count_parameters() {
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
