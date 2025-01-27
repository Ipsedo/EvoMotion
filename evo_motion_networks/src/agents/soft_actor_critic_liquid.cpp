//
// Created by samuel on 11/11/24.
//

#include <evo_motion_networks/agents/soft_actor_critic_liquid.h>
#include <evo_motion_networks/functions.h>
#include <evo_motion_networks/saver.h>

SoftActorCriticLiquidAgent::SoftActorCriticLiquidAgent(
    const int seed, const std::vector<int64_t> &state_space,
    const std::vector<int64_t> &action_space, int hidden_size, const int batch_size,
    const int epoch, float lr, const float gamma, const float tau, int unfolding_steps,
    const int replay_buffer_size, const int train_every)
    : actor(std::make_shared<ActorLiquidModule>(
          state_space, action_space, hidden_size, unfolding_steps)),
      critic_1(std::make_shared<QNetworkLiquidModule>(
          state_space, action_space, hidden_size, unfolding_steps)),
      critic_2(std::make_shared<QNetworkLiquidModule>(
          state_space, action_space, hidden_size, unfolding_steps)),
      target_critic_1(std::make_shared<QNetworkLiquidModule>(
          state_space, action_space, hidden_size, unfolding_steps)),
      target_critic_2(std::make_shared<QNetworkLiquidModule>(
          state_space, action_space, hidden_size, unfolding_steps)),
      actor_optimizer(std::make_shared<torch::optim::Adam>(actor->parameters(), lr)),
      critic_1_optimizer(std::make_shared<torch::optim::Adam>(critic_1->parameters(), lr)),
      critic_2_optimizer(std::make_shared<torch::optim::Adam>(critic_2->parameters(), lr)),
      target_entropy(-static_cast<float>(action_space[0])),
      entropy_parameter(std::make_shared<EntropyParameter>(1.f, 1)),
      entropy_optimizer(std::make_shared<torch::optim::Adam>(entropy_parameter->parameters(), lr)),
      curr_device(torch::kCPU), gamma(gamma), tau(tau), batch_size(batch_size), epoch(epoch),
      replay_buffer(replay_buffer_size, seed), curr_episode_step(0), curr_train_step(0L),
      global_curr_step(0L), actor_loss_meter("actor", 64), critic_1_loss_meter("critic_1", 64),
      critic_2_loss_meter("critic_2", 64), entropy_loss_meter("entropy", 64),
      episode_steps_meter("steps", 64), train_every(train_every) {

    hard_update(target_critic_1, critic_1);
    hard_update(target_critic_2, critic_2);

    set_eval(true);
}

torch::Tensor SoftActorCriticLiquidAgent::act(const torch::Tensor state, const float reward) {
    const liquid_sac_memory x_t{
        actor->get_x().detach(), critic_1->get_x().detach(), critic_2->get_x().detach(),
        target_critic_1->get_x().detach(), target_critic_2->get_x().detach()};

    const auto [mu, sigma] = actor->forward(state);
    const auto action = truncated_normal_sample(mu, sigma, -1.f, 1.f);

    const auto _1 = critic_1->forward(state, action);
    const auto _2 = critic_2->forward(state, action);
    const auto _3 = target_critic_1->forward(state, action);
    const auto _4 = target_critic_2->forward(state, action);

    const liquid_sac_memory next_x_t{
        actor->get_x().detach(), critic_1->get_x().detach(), critic_2->get_x().detach(),
        target_critic_1->get_x().detach(), target_critic_2->get_x().detach()};

    if (!replay_buffer.empty()) { replay_buffer.update_last(reward, state, false); }
    replay_buffer.add({{state, action.detach(), 0.f, false, state}, x_t, next_x_t});

    check_train();

    curr_episode_step++;
    global_curr_step++;

    return action;
}

void SoftActorCriticLiquidAgent::done(const torch::Tensor state, const float reward) {
    replay_buffer.update_last(reward, state, true);

    actor->reset_liquid();
    critic_1->reset_liquid();
    critic_2->reset_liquid();
    target_critic_1->reset_liquid();
    target_critic_2->reset_liquid();

    episode_steps_meter.add(static_cast<float>(curr_episode_step));
    curr_episode_step = 0;
}

void SoftActorCriticLiquidAgent::check_train() {
    if (global_curr_step % train_every == train_every - 1 && replay_buffer.has_enough(batch_size)) {

        set_eval(false);

        for (int e = 0; e < epoch; e++) {
            std::vector<liquid_episode_step<liquid_sac_memory>> tmp_replay_buffer =
                replay_buffer.sample(batch_size);

            std::vector<torch::Tensor> vec_states, vec_actions, vec_rewards, vec_done,
                vec_next_state, actor_x_t, critic_1_x_t, critic_2_x_t, target_critic_1_x_t,
                target_critic_2_x_t, actor_next_x_t, critic_1_next_x_t, critic_2_next_x_t,
                target_critic_1_next_x_t, target_critic_2_next_x_t;

            for (const auto &[step, x_t, next_x_t]: tmp_replay_buffer) {
                vec_states.push_back(step.state);
                vec_actions.push_back(step.action);
                vec_rewards.push_back(
                    torch::tensor({step.reward}, at::TensorOptions().device(curr_device)));
                vec_done.push_back(torch::tensor(
                    {step.done ? 1.f : 0.f}, at::TensorOptions().device(curr_device)));
                vec_next_state.push_back(step.next_state);

                actor_x_t.push_back(x_t.actor_x_t);
                critic_1_x_t.push_back(x_t.critic_1_x_t);
                critic_2_x_t.push_back(x_t.critic_2_x_t);
                target_critic_1_x_t.push_back(x_t.target_critic_1_x_t);
                target_critic_2_x_t.push_back(x_t.target_critic_2_x_t);

                actor_next_x_t.push_back(next_x_t.actor_x_t);
                critic_1_next_x_t.push_back(next_x_t.critic_1_x_t);
                critic_2_next_x_t.push_back(next_x_t.critic_2_x_t);
                target_critic_1_next_x_t.push_back(next_x_t.target_critic_1_x_t);
                target_critic_2_next_x_t.push_back(next_x_t.target_critic_2_x_t);
            }

            train(
                torch::stack(vec_states), torch::stack(vec_actions), torch::stack(vec_rewards),
                torch::stack(vec_done), torch::stack(vec_next_state),
                {torch::cat(actor_x_t), torch::cat(critic_1_x_t), torch::cat(critic_2_x_t),
                 torch::cat(target_critic_1_x_t), torch::cat(target_critic_2_x_t)},
                {torch::cat(actor_next_x_t), torch::cat(critic_1_next_x_t),
                 torch::cat(critic_2_next_x_t), torch::cat(target_critic_1_next_x_t),
                 torch::cat(target_critic_2_next_x_t)});
        }

        set_eval(true);
    }
}

void SoftActorCriticLiquidAgent::train(
    const torch::Tensor &batched_states, const torch::Tensor &batched_actions,
    const torch::Tensor &batched_rewards, const torch::Tensor &batched_done,
    const torch::Tensor &batched_next_state, const liquid_sac_memory &x_t,
    const liquid_sac_memory &next_x_t) {

    torch::Tensor target_q_values;
    {
        torch::NoGradGuard no_grad;

        const auto [next_mu, next_sigma, actor_next_next_x_t] =
            actor->forward(next_x_t.actor_x_t, batched_next_state);
        const auto next_action = truncated_normal_sample(next_mu, next_sigma, -1.f, 1.f);
        const auto next_log_prob =
            truncated_normal_log_pdf(next_action, next_mu, next_sigma, -1.f, 1.f).sum(-1, true);

        const auto [next_target_q_value_1, target_1_next_x_t] =
            target_critic_1->forward(next_x_t.target_critic_1_x_t, batched_next_state, next_action);
        const auto [next_target_q_value_2, target_2_next_x_t] =
            target_critic_2->forward(next_x_t.target_critic_2_x_t, batched_next_state, next_action);

        const auto target_v_value = torch::min(next_target_q_value_1, next_target_q_value_2)
                                    - entropy_parameter->alpha() * next_log_prob;

        target_q_values = (batched_rewards + (1.f - batched_done) * gamma * target_v_value);
    }

    // critic 1
    const auto [q_value_1, critic_1_next_x_t] =
        critic_1->forward(x_t.critic_1_x_t, batched_states, batched_actions);
    const auto critic_1_loss = torch::mse_loss(q_value_1, target_q_values, at::Reduction::Mean);

    critic_1_optimizer->zero_grad();
    critic_1_loss.backward();
    critic_1_optimizer->step();

    // critic 2
    const auto [q_value_2, critic_2_next_x_t] =
        critic_2->forward(x_t.critic_2_x_t, batched_states, batched_actions);
    const auto critic_2_loss = torch::mse_loss(q_value_2, target_q_values, at::Reduction::Mean);

    critic_2_optimizer->zero_grad();
    critic_2_loss.backward();
    critic_2_optimizer->step();

    // policy
    const auto [curr_mu, curr_sigma, actor_next_x_t] =
        actor->forward(x_t.actor_x_t, batched_states);
    const auto curr_action = truncated_normal_sample(curr_mu, curr_sigma, -1.f, 1.f);
    const auto curr_log_prob =
        truncated_normal_log_pdf(curr_action, curr_mu, curr_sigma, -1.f, 1.f).sum(-1, true);

    const auto [curr_q_value_1, _1] =
        critic_1->forward(x_t.critic_1_x_t, batched_states, curr_action);
    const auto [curr_q_value_2, _2] =
        critic_2->forward(x_t.critic_2_x_t, batched_states, curr_action);
    const auto q_value = torch::min(curr_q_value_1, curr_q_value_2);

    const auto actor_loss =
        torch::mean(entropy_parameter->alpha().detach() * curr_log_prob - q_value);

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

void SoftActorCriticLiquidAgent::save(const std::string &output_folder_path) {
    const std::filesystem::path path(output_folder_path);

    // actor
    save_torch(output_folder_path, actor, "actor.th");
    save_torch(output_folder_path, actor_optimizer, "actor_optimizer.th");

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

void SoftActorCriticLiquidAgent::load(const std::string &input_folder_path) {
    const std::filesystem::path path(input_folder_path);

    // actor
    load_torch(input_folder_path, actor, "actor.th");
    load_torch(input_folder_path, actor_optimizer, "actor_optimizer.th");

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

std::vector<LossMeter> SoftActorCriticLiquidAgent::get_metrics() {
    return {
        actor_loss_meter, critic_1_loss_meter, critic_2_loss_meter, entropy_loss_meter,
        episode_steps_meter};
}

void SoftActorCriticLiquidAgent::to(const torch::DeviceType device) {
    curr_device = device;
    actor->to(device);
    critic_1->to(device);
    critic_2->to(device);
    target_critic_1->to(device);
    target_critic_2->to(device);
    entropy_parameter->to(device);
}

void SoftActorCriticLiquidAgent::set_eval(const bool eval) {
    actor->train(!eval);
    critic_1->train(!eval);
    critic_2->train(!eval);
    target_critic_1->train(!eval);
    target_critic_2->train(!eval);
    entropy_parameter->train(!eval);
}

int SoftActorCriticLiquidAgent::count_parameters() {
    return count_module_parameters(actor) + count_module_parameters(target_critic_1)
           + count_module_parameters(target_critic_2) + count_module_parameters(critic_1)
           + count_module_parameters(critic_2) + count_module_parameters(entropy_parameter);
}
