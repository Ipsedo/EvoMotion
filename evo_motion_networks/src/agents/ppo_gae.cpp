//
// Created by samuel on 13/11/24.
//

#include <evo_motion_networks/agents/ppo_gae.h>
#include <evo_motion_networks/functions.h>
#include <evo_motion_networks/saver.h>

std::vector<torch::Tensor>
concat_vector(std::vector<torch::Tensor> v1, std::vector<torch::Tensor> v2) {
    std::vector<torch::Tensor> result(v1);
    result.insert(result.end(), v2.begin(), v2.end());
    return result;
}

PpoGaeAgent::PpoGaeAgent(
    const int seed, const std::vector<int64_t> &state_space,
    const std::vector<int64_t> &action_space, int hidden_size, const float gamma, const float lam,
    const float epsilon, const float entropy_factor, const float critic_loss_factor,
    const int epoch, const int batch_size, int train_every, int replay_buffer_size,
    float learning_rate, float clip_grad_norm)
    : actor(std::make_shared<ActorModule>(state_space, action_space, hidden_size)),
      critic(std::make_shared<CriticModule>(state_space, hidden_size)),
      optimizer(std::make_shared<torch::optim::Adam>(
          concat_vector(actor->parameters(), critic->parameters()), learning_rate)),
      gamma(gamma), lambda(lam), epsilon(epsilon), epoch(epoch), entropy_factor(entropy_factor),
      critic_loss_factor(critic_loss_factor), clip_grad_norm(clip_grad_norm), curr_train_step(0L),
      curr_episode_step(0L), global_curr_step(0L), batch_size(batch_size),
      replay_buffer(replay_buffer_size, seed), train_every(train_every),
      actor_loss_meter("actor_loss", 64), critic_loss_meter("critic_loss", 64),
      episode_steps_meter("steps", 64), curr_device(torch::kCPU) {

    auto params = actor->parameters();
    for (const auto &p: critic->parameters()) params.push_back(p);
    critic_optimizer = std::make_shared<torch::optim::Adam>(params, learning_rate);

    at::manual_seed(seed);
}

torch::Tensor PpoGaeAgent::act(const torch::Tensor state, const float reward) {
    set_eval(true);

    const auto [mu, sigma] = actor->forward(state);
    const auto action = truncated_normal_sample(mu, sigma, -1.0, 1.0);

    if (replay_buffer.empty()) replay_buffer.new_trajectory();
    if (!replay_buffer.trajectory_empty()) replay_buffer.update_last(reward, false, state);
    replay_buffer.add({state, action.detach(), 0.0, false, state});

    curr_episode_step++;

    return action;
}

void PpoGaeAgent::done(const torch::Tensor state, const float reward) {
    replay_buffer.update_last(reward, true, state);

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
                    torch::tensor({done ? 1.0 : 0.0}, torch::TensorOptions().device(curr_device)));
                vec_next_states.push_back(next_state);
            }

            const int pad = max_steps - static_cast<int>(trajectory.size());

            batch_vec_states.push_back(torch::pad(torch::stack(vec_states), {0, 0, 0, pad}));
            batch_vec_actions.push_back(torch::pad(torch::stack(vec_actions), {0, 0, 0, pad}));
            batch_vec_rewards.push_back(torch::pad(torch::stack(vec_rewards), {0, 0, 0, pad}));
            batch_vec_done.push_back(
                torch::pad(torch::stack(vec_done), {0, 0, 0, pad}, "constant", 1.0));
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

    //torch::autograd::DetectAnomalyGuard guard;

    set_eval(false);

    const auto [curr_values] = critic->forward(batched_states);
    const auto [next_values] = critic->forward(batched_next_state);

    const auto mask = torch::eq(
        torch::cat(
            {torch::ones({batched_done.size(0), 1, 1}, at::TensorOptions().device(curr_device)),
             torch::slice(1.0 - batched_done, 1, 0, batched_done.size(1) - 1)},
            1),
        1.0);

    const auto deltas = batched_rewards + (1.0 - batched_done) * gamma * next_values - curr_values;

    auto gae_step =
        torch::zeros({batched_states.size(0), 1}, at::TensorOptions().device(curr_device));
    std::vector<torch::Tensor> advantages_vec;

    for (auto t = batched_rewards.size(1) - 1; t >= 0; t--) {
        gae_step = torch::select(deltas, 1, t) * torch::select(mask, 1, t)
                   + gamma * lambda * (1.0 - torch::select(batched_done, 1, t)) * gae_step;
        advantages_vec.push_back(gae_step);
    }

    auto advantages = torch::stack(advantages_vec, 1).flip({1});
    advantages = (advantages - torch::masked_select(advantages, mask).mean())
                 / (torch::masked_select(advantages, mask).std() + 1e-8);

    const auto returns = advantages + curr_values;

    const auto [old_mu, old_sigma] = actor->forward(batched_states);
    const auto old_log_prob =
        truncated_normal_log_pdf(batched_actions, old_mu, old_sigma, -1.0, 1.0);

    for (int i = 0; i < epoch; i++) {
        const auto [mu, sigma] = actor->forward(batched_states);
        const auto log_prob = truncated_normal_log_pdf(batched_actions, mu, sigma, -1.0, 1.0);
        const auto entropy = truncated_normal_entropy(mu, sigma, -1.0, 1.0);

        const auto [value] = critic->forward(batched_states);

        // actor
        const auto ratios = torch::exp(log_prob - old_log_prob.detach());

        const auto surrogate_1 = ratios * advantages.detach();
        const auto surrogate_2 =
            torch::clamp(ratios, 1.0 - epsilon, 1.0 + epsilon) * advantages.detach();

        const auto actor_loss =
            -torch::sum(torch::min(surrogate_1, surrogate_2) + entropy_factor * entropy, -1, true);

        // critic
        const auto critic_loss = critic_loss_factor * torch::pow(value - returns.detach(), 2.0);

        // optimize
        const auto loss = torch::mean(actor_loss + critic_loss);

        optimizer->zero_grad();
        loss.backward();
        torch::nn::utils::clip_grad_norm_(critic->parameters(), clip_grad_norm);
        torch::nn::utils::clip_grad_norm_(actor->parameters(), clip_grad_norm);
        optimizer->step();

        actor_loss_meter.add(actor_loss.mean().item().toFloat());
        critic_loss_meter.add(critic_loss.mean().item().toFloat());
    }

    curr_train_step++;
}

void PpoGaeAgent::save(const std::string &output_folder_path) {
    save_torch(output_folder_path, actor, "actor.th");
    save_torch(output_folder_path, critic, "critic.th");
    save_torch(output_folder_path, optimizer, "optimizer.th");
}

void PpoGaeAgent::load(const std::string &input_folder_path) {
    load_torch(input_folder_path, actor, "actor.th");
    load_torch(input_folder_path, critic, "critic.th");
    load_torch(input_folder_path, optimizer, "optimizer.th");
}

std::vector<LossMeter> PpoGaeAgent::get_metrics() {
    return {actor_loss_meter, critic_loss_meter, episode_steps_meter};
}

void PpoGaeAgent::to(const torch::DeviceType device) {
    curr_device = device;
    actor->to(device);
    critic->to(device);
}

void PpoGaeAgent::set_eval(const bool eval) {
    actor->train(!eval);
    critic->train(!eval);
}

int PpoGaeAgent::count_parameters() {
    return count_module_parameters(actor) + count_module_parameters(critic);
}
