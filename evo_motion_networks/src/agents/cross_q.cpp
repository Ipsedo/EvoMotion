//
// Created by samuel on 09/01/25.
//

#include <evo_motion_networks/agents/cross_q.h>
#include <evo_motion_networks/functions.h>
#include <evo_motion_networks/saver.h>

CrossQAgent::CrossQAgent(
    int seed, const std::vector<int64_t> &state_space, const std::vector<int64_t> &action_space,
    int actor_hidden_size, int critic_hidden_size, int batch_size, int epoch, float lr, float gamma,
    int replay_buffer_size, int train_every)
    : actor(std::make_shared<ActorModule>(state_space, action_space, actor_hidden_size)),
      critic_1(
          std::make_shared<BatchNormQNetworkModule>(state_space, action_space, critic_hidden_size)),
      critic_2(
          std::make_shared<BatchNormQNetworkModule>(state_space, action_space, critic_hidden_size)),
      actor_optimizer(std::make_shared<torch::optim::Adam>(
          actor->parameters(), torch::optim::AdamOptions(lr).betas({0.5, 0.999}))),
      critic_1_optimizer(std::make_shared<torch::optim::Adam>(
          critic_1->parameters(), torch::optim::AdamOptions(lr).betas({0.5, 0.999}))),
      critic_2_optimizer(std::make_shared<torch::optim::Adam>(
          critic_2->parameters(), torch::optim::AdamOptions(lr).betas({0.5, 0.999}))),
      target_entropy(-static_cast<float>(action_space[0])),
      entropy_parameter(std::make_shared<EntropyParameter>(1.f, 1)),
      entropy_optimizer(std::make_shared<torch::optim::Adam>(
          entropy_parameter->parameters(), torch::optim::AdamOptions(lr).betas({0.5, 0.999}))),
      curr_device(torch::kCPU), gamma(gamma), batch_size(batch_size), epoch(epoch),
      train_every(train_every), replay_buffer(replay_buffer_size, seed), curr_episode_step(0),
      curr_train_step(0L), global_curr_step(0L), actor_loss_meter("actor", 64),
      critic_1_loss_meter("critic_1", 64), critic_2_loss_meter("critic_2", 64),
      entropy_loss_meter("entropy", 64), episode_steps_meter("steps", 64),
      reward_meter("rewards", 64) {
    at::manual_seed(seed);
    set_eval(true);
}

void CrossQAgent::train(
    const torch::Tensor &batched_states, const torch::Tensor &batched_actions,
    const torch::Tensor &batched_rewards, const torch::Tensor &batched_done,
    const torch::Tensor &batched_next_state) {

    set_eval(false);

    // prepared concatenated states and actions
    const auto [next_mu, next_sigma] = actor->forward(batched_next_state);
    const auto next_action = truncated_normal_sample(next_mu, next_sigma, -1.f, 1.f);
    const auto next_log_proba =
        truncated_normal_log_pdf(next_action, next_mu, next_sigma, -1.f, 1.f).sum(-1, true);

    const auto cat_states = torch::cat({batched_states, batched_next_state}, 0);
    const auto cat_actions = torch::cat({batched_actions, next_action.detach()}, 0);

    // compute q-value and next q-value
    const auto [cat_q_value_1] = critic_1->forward(cat_states, cat_actions);
    const auto [cat_q_value_2] = critic_2->forward(cat_states, cat_actions);

    const auto chunk_cat_q_value_1 = torch::chunk(cat_q_value_1, 2, 0);
    const auto &q_value_1 = chunk_cat_q_value_1[0];
    const auto &next_q_value_1 = chunk_cat_q_value_1[1];

    const auto chunk_cat_q_value_2 = torch::chunk(cat_q_value_2, 2, 0);
    const auto &q_value_2 = chunk_cat_q_value_2[0];
    const auto &next_q_value_2 = chunk_cat_q_value_2[1];

    // compute target q value
    const auto target_v_value =
        torch::min(next_q_value_1, next_q_value_2) - entropy_parameter->alpha() * next_log_proba;
    const auto target_q_values =
        batched_rewards + (1.f - batched_done) * gamma * target_v_value.detach();

    // optimize critic 1
    const auto critic_1_loss = torch::mse_loss(q_value_1, target_q_values);

    critic_1_optimizer->zero_grad();
    critic_1_loss.backward();
    critic_1_optimizer->step();

    // optimize critic 2
    const auto critic_2_loss = torch::mse_loss(q_value_2, target_q_values);

    critic_2_optimizer->zero_grad();
    critic_2_loss.backward();
    critic_2_optimizer->step();

    // optimize actor
    const auto [curr_mu, curr_sigma] = actor->forward(batched_states);
    const auto curr_action = truncated_normal_sample(curr_mu, curr_sigma, -1.f, 1.f);
    const auto curr_log_proba =
        truncated_normal_log_pdf(curr_action, curr_mu, curr_sigma, -1.f, 1.f).sum(-1, true);

    critic_1->train(false);
    critic_2->train(false);

    const auto [curr_q_value_1] = critic_1->forward(batched_states, curr_action);
    const auto [curr_q_value_2] = critic_2->forward(batched_states, curr_action);
    const auto curr_q_value = torch::min(curr_q_value_1, curr_q_value_2);

    const auto actor_loss =
        torch::mean(entropy_parameter->alpha().detach() * curr_log_proba - curr_q_value);

    actor_optimizer->zero_grad();
    actor_loss.backward();
    actor_optimizer->step();

    // optimize entropy
    const auto entropy_loss =
        -torch::mean(entropy_parameter->log_alpha() * (curr_log_proba.detach() + target_entropy));

    entropy_optimizer->zero_grad();
    entropy_loss.backward();
    entropy_optimizer->step();

    // eval
    set_eval(true);

    // metrics
    actor_loss_meter.add(actor_loss.cpu().item().toFloat());
    critic_1_loss_meter.add(critic_1_loss.cpu().item().toFloat());
    critic_2_loss_meter.add(critic_2_loss.cpu().item().toFloat());
    entropy_loss_meter.add(entropy_loss.cpu().item().toFloat());

    curr_train_step++;
}

void CrossQAgent::check_train() {
    if (global_curr_step % train_every == train_every - 1 && replay_buffer.has_enough(batch_size)) {

        for (int e = 0; e < epoch; e++) {
            std::vector<episode_step> tmp_replay_buffer = replay_buffer.sample(batch_size);

            std::vector<torch::Tensor> vec_states, vec_actions, vec_rewards, vec_done,
                vec_next_state;

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
                torch::stack(vec_states), torch::stack(vec_actions), torch::stack(vec_rewards),
                torch::stack(vec_done), torch::stack(vec_next_state));
        }
    }
}

torch::Tensor CrossQAgent::act(torch::Tensor state, float reward) {
    set_eval(true);

    const auto [mu, sigma] = actor->forward(state);
    const auto action = truncated_normal_sample(mu, sigma, -1.f, 1.f);

    if (!replay_buffer.empty()) { replay_buffer.update_last(reward, state, false); }
    replay_buffer.add({state, action.detach(), 0.f, false, state});

    check_train();

    curr_episode_step++;
    global_curr_step++;

    return action;
}

void CrossQAgent::done(torch::Tensor state, float reward) {
    replay_buffer.update_last(reward, state, true);

    reward_meter.add(reward);
    episode_steps_meter.add(static_cast<float>(curr_episode_step));

    curr_episode_step = 0;
}

void CrossQAgent::save(const std::string &output_folder_path) {
    const std::filesystem::path path(output_folder_path);

    // actor
    save_torch(output_folder_path, actor, "actor.th");
    save_torch(output_folder_path, actor_optimizer, "actor_optimizer.th");

    // critic
    save_torch(output_folder_path, critic_1, "critic_1.th");
    save_torch(output_folder_path, critic_1_optimizer, "critic_1_optimizer.th");

    save_torch(output_folder_path, critic_2, "critic_2.th");
    save_torch(output_folder_path, critic_2_optimizer, "critic_2_optimizer.th");

    // Entropy
    save_torch(output_folder_path, entropy_parameter, "entropy.th");
    save_torch(output_folder_path, entropy_optimizer, "entropy_optimizer.th");
}

void CrossQAgent::load(const std::string &input_folder_path) {
    const std::filesystem::path path(input_folder_path);

    // actor
    load_torch(input_folder_path, actor, "actor.th");
    load_torch(input_folder_path, actor_optimizer, "actor_optimizer.th");

    // critic
    load_torch(input_folder_path, critic_1, "critic_1.th");
    load_torch(input_folder_path, critic_1_optimizer, "critic_1_optimizer.th");

    load_torch(input_folder_path, critic_2, "critic_2.th");
    load_torch(input_folder_path, critic_2_optimizer, "critic_2_optimizer.th");

    // Entropy
    load_torch(input_folder_path, entropy_parameter, "entropy.th");
    load_torch(input_folder_path, entropy_optimizer, "entropy_optimizer.th");
}

std::vector<LossMeter> CrossQAgent::get_metrics() {
    return {actor_loss_meter,   critic_1_loss_meter, critic_2_loss_meter,
            entropy_loss_meter, episode_steps_meter, reward_meter};
}

void CrossQAgent::to(torch::DeviceType device) {
    curr_device = device;
    actor->to(device);
    critic_1->to(device);
    critic_2->to(device);
    entropy_parameter->to(device);
}

void CrossQAgent::set_eval(bool eval) {
    actor->train(!eval);
    critic_1->train(!eval);
    critic_2->train(!eval);
    entropy_parameter->train(!eval);
}

int CrossQAgent::count_parameters() {
    return count_module_parameters(actor) + count_module_parameters(critic_1)
           + count_module_parameters(critic_2) + count_module_parameters(entropy_parameter);
}
