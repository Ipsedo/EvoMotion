//
// Created by samuel on 08/11/24.
//

#include <filesystem>

#include <evo_motion_networks/agents/soft_actor_critic.h>
#include <evo_motion_networks/functions.h>
#include <evo_motion_networks/networks/actor.h>
#include <evo_motion_networks/saver.h>

/*
 * Agents
 */

SoftActorCriticAgent::SoftActorCriticAgent(
    const int seed, const std::vector<int64_t> &state_space,
    const std::vector<int64_t> &action_space, int hidden_size, const int batch_size, float lr,
    const float gamma, const float tau, const int replay_buffer_size, const int train_every)
    : actor(std::make_shared<ActorModule>(state_space, action_space, hidden_size)),
      critic_1(std::make_shared<QNetworkModule>(state_space, action_space, hidden_size)),
      critic_2(std::make_shared<QNetworkModule>(state_space, action_space, hidden_size)),
      target_critic_1(std::make_shared<QNetworkModule>(state_space, action_space, hidden_size)),
      target_critic_2(std::make_shared<QNetworkModule>(state_space, action_space, hidden_size)),
      actor_optimizer(std::make_shared<torch::optim::Adam>(actor->parameters(), lr)),
      critic_1_optimizer(std::make_shared<torch::optim::Adam>(critic_1->parameters(), lr)),
      critic_2_optimizer(std::make_shared<torch::optim::Adam>(critic_2->parameters(), lr)),
      target_entropy(-1.f), entropy_parameter(std::make_shared<EntropyParameter>(action_space[0])),
      entropy_optimizer(std::make_shared<torch::optim::Adam>(entropy_parameter->parameters(), lr)),
      curr_device(torch::kCPU), gamma(gamma), tau(tau), batch_size(batch_size),
      replay_buffer(replay_buffer_size, seed), curr_episode_step(0), curr_train_step(0L),
      global_curr_step(0L), actor_loss_meter("actor", 16), critic_1_loss_meter("critic_1", 16),
      critic_2_loss_meter("critic_2", 16), entropy_loss_meter("entropy", 16),
      episode_steps_meter("steps", 16), train_every(train_every) {
    at::manual_seed(seed);

    hard_update(target_critic_1, critic_1);
    hard_update(target_critic_2, critic_2);
}

torch::Tensor SoftActorCriticAgent::act(const torch::Tensor state, const float reward) {
    const auto [mu, sigma] = actor->forward(state);
    const auto action = truncated_normal_sample(mu, sigma, -1.f, 1.f);

    if (!replay_buffer.empty()) { replay_buffer.update_last(reward, state, false); }
    replay_buffer.add({state, action.detach(), 0.f, false, state});

    check_train();

    curr_episode_step++;
    global_curr_step++;

    return action;
}

void SoftActorCriticAgent::check_train() {
    if (global_curr_step % train_every == train_every - 1) {
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

void SoftActorCriticAgent::train(
    const torch::Tensor &batched_states, const torch::Tensor &batched_actions,
    const torch::Tensor &batched_rewards, const torch::Tensor &batched_done,
    const torch::Tensor &batched_next_state) {

    const auto [next_mu, next_sigma] = actor->forward(batched_next_state);
    const auto next_action = truncated_normal_sample(next_mu, next_sigma, -1.f, 1.f);
    const auto next_log_prob =
        truncated_normal_log_pdf(next_action, next_mu, next_sigma, -1.f, 1.f).sum(-1, true);

    const auto [next_target_q_value_1] = target_critic_1->forward(batched_next_state, next_action);
    const auto [next_target_q_value_2] = target_critic_2->forward(batched_next_state, next_action);

    const auto target_v_value =
        torch::min(next_target_q_value_1, next_target_q_value_2) - entropy_parameter->alpha() * next_log_prob;
    const auto norm_rewards = (batched_rewards - batched_rewards.mean()) / (batched_rewards.std() + 1e-8);
    const auto target_q_values = (norm_rewards + (1.f - batched_done) * gamma * target_v_value).detach();

    // critic 1
    const auto [q_value_1] = critic_1->forward(batched_states, batched_actions);
    const auto critic_1_loss = torch::mse_loss(q_value_1, target_q_values, at::Reduction::Mean);

    critic_1_optimizer->zero_grad();
    critic_1_loss.backward();
    critic_1_optimizer->step();

    // critic 2
    const auto [q_value_2] = critic_2->forward(batched_states, batched_actions);
    const auto critic_2_loss = torch::mse_loss(q_value_2, target_q_values, at::Reduction::Mean);

    critic_2_optimizer->zero_grad();
    critic_2_loss.backward();
    critic_2_optimizer->step();

    // policy
    const auto [curr_mu, curr_sigma] = actor->forward(batched_states);
    const auto curr_action = truncated_normal_sample(curr_mu, curr_sigma, -1.f, 1.f);
    const auto curr_log_prob =
        truncated_normal_log_pdf(curr_action.detach(), curr_mu, curr_sigma, -1.f, 1.f).sum(-1, true);

    const auto [curr_q_value_1] = critic_1->forward(batched_states, curr_action);
    const auto [curr_q_value_2] = critic_2->forward(batched_states, curr_action);
    const auto q_value = torch::min(curr_q_value_1, curr_q_value_2);

    const auto actor_loss = torch::mean(entropy_parameter->alpha().detach() * curr_log_prob - q_value);

    actor_optimizer->zero_grad();
    actor_loss.backward();
    actor_optimizer->step();

    // entropy
    const auto entropy_loss =
        -torch::mean(entropy_parameter->log_alpha() * (curr_log_prob.detach() + target_entropy));

    entropy_optimizer->zero_grad();
    entropy_loss.backward();
    entropy_optimizer->step();

    // target value soft update
    soft_update(target_critic_1, critic_1, tau);
    soft_update(target_critic_2, critic_2, tau);

    // metrics
    actor_loss_meter.add(actor_loss.cpu().item().toFloat());
    critic_1_loss_meter.add(critic_1_loss.cpu().item().toFloat());
    critic_2_loss_meter.add(critic_2_loss.cpu().item().toFloat());
    entropy_loss_meter.add(entropy_loss.cpu().item().toFloat());

    curr_train_step++;
}

void SoftActorCriticAgent::done(const torch::Tensor state, const float reward) {
    replay_buffer.update_last(reward, state, true);

    episode_steps_meter.add(static_cast<float>(curr_episode_step));
    curr_episode_step = 0;
}

void SoftActorCriticAgent::save(const std::string &output_folder_path) {
    const std::filesystem::path path(output_folder_path);

    // actor
    save_torch(output_folder_path, actor, "actor.th");
    save_torch(output_folder_path, actor_optimizer, "actor.th");

    // critic
    save_torch(output_folder_path, critic_1, "critic_1.th");
    save_torch(output_folder_path, target_critic_1, "target_critic_1.th");
    save_torch(output_folder_path, critic_1_optimizer, "critic_1_optimizer.th");

    save_torch(output_folder_path, critic_2, "critic_2.th");
    save_torch(output_folder_path, target_critic_2, "target_critic_2.th");
    save_torch(output_folder_path, critic_2_optimizer, "critic_2_optimizer.th");

    // Entropy
    save_torch(output_folder_path, entropy_parameter, "entropy.th");
    save_torch(output_folder_path, entropy_optimizer, "entropy_optimizer.th");
}

void SoftActorCriticAgent::load(const std::string &input_folder_path) {
    const std::filesystem::path path(input_folder_path);

    // actor
    load_torch(input_folder_path, actor, "actor.th");
    load_torch(input_folder_path, actor_optimizer, "actor.th");

    // critic
    load_torch(input_folder_path, critic_1, "critic_1.th");
    load_torch(input_folder_path, target_critic_1, "target_critic_1.th");
    load_torch(input_folder_path, critic_1_optimizer, "critic_1_optimizer.th");

    load_torch(input_folder_path, critic_2, "critic_2.th");
    load_torch(input_folder_path, target_critic_2, "target_critic_2.th");
    load_torch(input_folder_path, critic_2_optimizer, "critic_2_optimizer.th");

    // Entropy
    load_torch(input_folder_path, entropy_parameter, "entropy.th");
    load_torch(input_folder_path, entropy_optimizer, "entropy_optimizer.th");
}

std::vector<LossMeter> SoftActorCriticAgent::get_metrics() {
    return {
        actor_loss_meter, critic_1_loss_meter, critic_2_loss_meter, entropy_loss_meter,
        episode_steps_meter};
}

void SoftActorCriticAgent::to(torch::DeviceType device) {
    curr_device = device;
    actor->to(device);
    critic_1->to(device);
    critic_2->to(device);
    target_critic_1->to(device);
    target_critic_2->to(device);
    entropy_parameter->to(device);
}

void SoftActorCriticAgent::set_eval(bool eval) {
    if (eval) {
        actor->eval();
        critic_1->eval();
        critic_2->eval();
        target_critic_1->eval();
        target_critic_2->eval();
        entropy_parameter->eval();
    } else {
        actor->train();
        critic_1->train();
        critic_2->train();
        target_critic_1->train();
        target_critic_2->train();
        entropy_parameter->train();
    }
}

int SoftActorCriticAgent::count_parameters() {
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
