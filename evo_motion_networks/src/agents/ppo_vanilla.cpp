//
// Created by samuel on 21/11/24.
//

#include <evo_motion_networks/agents/ppo_gae.h>
#include <evo_motion_networks/agents/ppo_vanilla.h>
#include <evo_motion_networks/functions.h>
#include <evo_motion_networks/saver.h>

PpoVanillaAgent::PpoVanillaAgent(
    const int seed, const std::vector<int64_t> &state_space,
    const std::vector<int64_t> &action_space, int hidden_size, const float gamma,
    const float epsilon, const float entropy_factor, const float critic_loss_factor,
    const int epoch, const int batch_size, float learning_rate)
    : actor(std::make_shared<ActorModule>(state_space, action_space, hidden_size)),
      actor_optimizer(std::make_shared<torch::optim::Adam>(actor->parameters(), learning_rate)),
      critic(std::make_shared<CriticModule>(state_space, hidden_size)),
      critic_optimizer(std::make_shared<torch::optim::Adam>(critic->parameters(), learning_rate)),
      gamma(gamma), epsilon(epsilon), epoch(epoch), entropy_factor(entropy_factor),
      critic_loss_factor(critic_loss_factor), curr_train_step(0L), curr_episode_step(0L),
      global_curr_step(0L), batch_size(batch_size), replay_buffer(batch_size, seed),
      train_every(batch_size), actor_loss_meter("actor_loss", 64),
      critic_loss_meter("critic_loss", 64), episode_steps_meter("steps", 64),
      curr_device(torch::kCPU) {

    at::manual_seed(seed);

    set_eval(true);
}

torch::Tensor PpoVanillaAgent::act(const torch::Tensor state, const float reward) {
    const auto [mu, sigma] = actor->forward(state);
    auto action = truncated_normal_sample(mu, sigma, -1.f, 1.f);

    if (!replay_buffer.empty()) { replay_buffer.update_last(reward, state, false); }
    replay_buffer.add({state, action.detach(), 0.f, false, state});

    check_train();

    curr_episode_step++;
    global_curr_step++;

    return action;
}

void PpoVanillaAgent::done(const torch::Tensor state, const float reward) {
    replay_buffer.update_last(reward, state, true);

    episode_steps_meter.add(static_cast<float>(curr_episode_step));
    curr_episode_step = 0;
}

void PpoVanillaAgent::check_train() {
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

void PpoVanillaAgent::train(
    const torch::Tensor &batched_states, const torch::Tensor &batched_actions,
    const torch::Tensor &batched_rewards, const torch::Tensor &batched_done,
    const torch::Tensor &batched_next_state) {

    set_eval(false);

    const auto [curr_values] = critic->forward(batched_states);
    const auto [next_values] = critic->forward(batched_next_state);

    auto advantages = batched_rewards + (1.f - batched_done) * gamma * next_values - curr_values;
    advantages = (advantages - advantages.mean()) / (advantages.std() + 1e-8);
    const auto target = advantages + curr_values;

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

        const auto actor_loss = -torch::mean(
            torch::sum(torch::min(surrogate_1, surrogate_2) + entropy_factor * entropy, -1));

        actor_optimizer->zero_grad();
        actor_loss.backward();
        actor_optimizer->step();

        // critic
        const auto critic_loss =
            critic_loss_factor * torch::mse_loss(value, target.detach(), torch::Reduction::Mean);

        critic_optimizer->zero_grad();
        critic_loss.backward();
        critic_optimizer->step();

        actor_loss_meter.add(actor_loss.item().toFloat());
        critic_loss_meter.add(critic_loss.item().toFloat());
    }

    set_eval(true);

    curr_train_step++;
}

void PpoVanillaAgent::save(const std::string &output_folder_path) {
    save_torch(output_folder_path, actor, "actor.th");
    save_torch(output_folder_path, actor_optimizer, "actor_optimizer.th");
    save_torch(output_folder_path, critic, "critic.th");
    save_torch(output_folder_path, critic_optimizer, "critic_optimizer");
}

void PpoVanillaAgent::load(const std::string &input_folder_path) {
    load_torch(input_folder_path, actor, "actor.th");
    load_torch(input_folder_path, actor_optimizer, "actor_optimizer.th");
    load_torch(input_folder_path, critic, "critic.th");
    load_torch(input_folder_path, critic_optimizer, "critic_optimizer");
}

std::vector<LossMeter> PpoVanillaAgent::get_metrics() {
    return {actor_loss_meter, critic_loss_meter, episode_steps_meter};
}

void PpoVanillaAgent::to(const torch::DeviceType device) {
    curr_device = device;
    actor->to(device);
    critic->to(device);
}

void PpoVanillaAgent::set_eval(const bool eval) {
    actor->train(!eval);
    critic->train(!eval);
}

int PpoVanillaAgent::count_parameters() {
    return count_module_parameters(actor) + count_module_parameters(critic);
}
